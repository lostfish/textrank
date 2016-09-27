#include <iostream>
#include <algorithm>
#include <math.h>
#include <cctype>
#include "text_rank.h"

using namespace std;

#define _DEBUG

void TextRank::ExtractHighTfWords(const vector<string> &token_vec, vector<pair<string, double> > &keywords, const size_t topN)
{
    keywords.clear();
    if (token_vec.empty())
        return;
    map<string, double> word_tf;
    for (size_t i = 0; i < token_vec.size(); ++i)
    {
        const string &word = token_vec[i];
        if (word_tf.find(word) == word_tf.end())
            word_tf[word] = 1;
        else
            word_tf[word] += 1;
    }
    vector<pair<string, double> > word_tf_vec(word_tf.begin(), word_tf.end());
    sort(word_tf_vec.begin(),word_tf_vec.end(), value_great());
    for (size_t i = 0; i < word_tf_vec.size(); ++i)
    {
        if (i == topN)
            break;
        keywords.push_back(word_tf_vec[i]);
    }
}

void TextRank::ExtractKeyword(const vector<string> &token_vec, vector<pair<string, double> > &keywords, const size_t topN)
{
    keywords.clear();
    if (token_vec.empty())
        return;

    Clear();

    map<size_t, set<size_t> > word_neighbors;
    map<size_t, double> score_map;

    BuildWordRelation(token_vec, word_neighbors);

#ifdef _DEBUG
    cout << "token num: " << token_vec.size() << '\t' << word_neighbors.size() << endl;
#endif

    CalcWordScore(word_neighbors, score_map);

    vector<pair<size_t, double> > score_vec(score_map.begin(), score_map.end());
    sort(score_vec.begin(), score_vec.end(), value_great());

    for (size_t i = 0; i < score_vec.size(); ++i)
    {
        if ( i == topN )
            break;
        const string &word = m_word_vec[score_vec[i].first]; 
        keywords.push_back(make_pair(word, score_vec[i].second));
    }
}

void TextRank::UpdateWeightMap(size_t i, size_t j)
{
    if (i > j)
    {
        size_t tmp = i;
        i = j;
        j = tmp;
    }
    pair<size_t, size_t> key(i, j);
    if (m_weight_map.find(key) != m_weight_map.end())
        m_weight_map[key] += 1;
    else
        m_weight_map.insert(make_pair(key, 1.0));
}

double TextRank::GetWeight(size_t i, size_t j) const
{
    if (i > j)
    {
        size_t tmp = i;
        i = j;
        j = tmp;
    }
    pair<size_t, size_t> key(i, j);
    if (m_weight_map.find(key) != m_weight_map.end())
        return m_weight_map.at(key);
    return 0;
}

int TextRank::BuildWordRelation(const vector<string> &token_vec, map<size_t, set<size_t> > &word_neighbors)
{
    m_weight_map.clear();
    word_neighbors.clear();

    const size_t n = token_vec.size();
    
    // use word index to replace string word 
    for(size_t i = 0; i < n; ++i)
    {
        const string &word = token_vec[i];
        if (m_word_index.find(word) == m_word_index.end())
        {
            m_word_vec.push_back(word);
            m_word_index.insert(make_pair(word, m_word_vec.size()-1));
        }
    }

    for(size_t i = 0; i < n; ++i)
    {
        const string &word = token_vec[i];
        size_t id1 = m_word_index.at(word);

        if (word_neighbors.find(id1) == word_neighbors.end())
            word_neighbors.insert(make_pair(id1, set<size_t>()));

        for(size_t j = 1; j < m_window_length; ++j)
        {
            if (i+j >= n)
                break;
            size_t id2 = m_word_index.at(token_vec[i+j]);
            if (id2 == id1)
                continue;

            UpdateWeightMap(id1, id2);

            word_neighbors[id1].insert(id2);

            // undirected graph
            if (word_neighbors.find(id2) == word_neighbors.end())
                word_neighbors.insert(make_pair(id2, set<size_t>()));
            word_neighbors[id2].insert(id1);
        }
    }

    // calc weight sum of out-edges
    map<size_t, set<size_t> >::iterator witer = word_neighbors.begin();
    for (; witer != word_neighbors.end(); ++witer)
    {
        size_t id1 = witer->first;
        set<size_t> &neighbors = witer->second;
        set<size_t>::iterator niter = neighbors.begin();
        double sum = 0;
        for (; niter != neighbors.end(); ++niter)
        {
            size_t id2 = *niter;
            sum += GetWeight(id1, id2);
        }
        m_out_sum_map.insert(make_pair(id1, sum));
    }
    return 0;
}


void TextRank::CalcWordScore(const map<size_t, set<size_t> > &word_neighbors, map<size_t, double> &score_map)
{
    score_map.clear();

    // initialize
    map<size_t, set<size_t> >::const_iterator witer = word_neighbors.begin();
    for (; witer != word_neighbors.end(); ++witer)
        score_map.insert(make_pair(witer->first, 1.0));

    // iterate
    for (size_t i = 0; i < m_max_iter_num; ++i)
    {
        double max_delta = 0;
        map<size_t, double> new_score_map; // current iteration score

        for (witer = word_neighbors.begin(); witer != word_neighbors.end(); ++witer)
        {
            size_t id1 = witer->first;
            double new_score = 1 - m_d;
            double sum_weight = 0;
            const set<size_t> &neighbors = witer->second;
            set<size_t>::const_iterator niter = neighbors.begin();
            for (; niter != neighbors.end(); ++niter)
            {
                size_t id2 = *niter;
                double weight = GetWeight(id2, id1);
                sum_weight += weight/m_out_sum_map.at(id2)*score_map[id2];
                //sum_weight +=  1.0/word_neighbors[id2].size()*score_map[id2];
            }
            new_score += m_d * sum_weight;
            new_score_map.insert(make_pair(id1, new_score));

            double delta = fabs(new_score - score_map[id1]);
            max_delta = max(max_delta, delta);
#ifdef _DEBUG
            //cout << "iter " << i << '\t' << m_word_vec[id1] << '\t' << new_score << '\t' << delta << endl;
#endif
        }
        score_map = new_score_map;
        if (max_delta < m_least_delta)
        {
#ifdef _DEBUG
            cout << "stop iteration when iter = " << i << '\t' << max_delta << endl; 
#endif
            break;
        }
#ifdef _DEBUG
        //cout << "iter: " << i << '\t' << max_delta << endl; 
        if (i == m_max_iter_num - 1)
            cout << "max_delta: " << max_delta << endl; 
#endif
    }
}

void TextRank::Clear()
{
    m_word_vec.clear();
    m_word_index.clear();
    m_weight_map.clear();
    m_out_sum_map.clear();
}
