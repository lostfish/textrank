#include <iostream>
#include <algorithm>
#include <math.h>
#include <cctype>
#include <iterator> // advance
#include "sentence_rank.h"

using namespace std;

#define _DEBUG

void SentenceRank::ExtractKeySentence(const map<string, vector<string> > &sentence_token_map, vector<pair<string, double> > &key_sentences, const size_t topN )
{
    key_sentences.clear();

    size_t n = sentence_token_map.size();
    
    if (n == 0)
        return;

    if (n == 1)
    {
        key_sentences.push_back(make_pair(sentence_token_map.begin()->first, 1.0));
        return;
    }

    Clear();

    BuildSentenceRelation(sentence_token_map);

    map<size_t, double> score_map;
    CalcSentenceScore(score_map);

    vector<pair<size_t, double> > score_vec(score_map.begin(), score_map.end());
    sort(score_vec.begin(), score_vec.end(), value_great());

    for (size_t i = 0; i < score_vec.size(); ++i)
    {
        if ( i == topN )
            break;
        const string &sent = m_sentence_vec[score_vec[i].first]; 
        key_sentences.push_back(make_pair(sent, score_vec[i].second));
    }

}

double SentenceRank::CalcDist(const vector<string> &token_vec1, const vector<string> &token_vec2) 
{
    size_t both_num = 0;
    size_t n1 = token_vec1.size();
    size_t n2 = token_vec2.size();
    if (n1 < 2 || n2 < 2)
        return 0;
    for (size_t i = 0; i < n1; ++i)
    {
        const string &token = token_vec1[i];
        for(size_t j = 0; j < n2; ++j)
        {
            if (token == token_vec2[j])
            {
                both_num++;
                break;
            }
        }
    }
    double dist = both_num / ( log(n1) + log(n2) );
    return dist;
}

double SentenceRank::GetWeight(size_t i, size_t j) const
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

void SentenceRank::BuildSentenceRelation(const map<string, vector<string> > &sentence_token_map)
{
    map<string, vector<string> >::const_iterator iter = sentence_token_map.begin();
    for (; iter != sentence_token_map.end(); ++iter)
    {
        m_sentence_vec.push_back(iter->first);
        size_t i = m_sentence_vec.size() - 1;
        m_sentence_index.insert(make_pair(iter->first, i));
        size_t j = i + 1;
        map<string, vector<string> >::const_iterator iter2 = iter;
        advance(iter2, 1);
        for (; iter2 != sentence_token_map.end(); ++iter2)
        {
            double dist = CalcDist(iter->second, iter2->second);
            pair<size_t, size_t> key(i, j);
            m_weight_map.insert(make_pair(key, dist));
            j++;
        }
    }
    // calc weight sum of out-edges
    size_t n = m_sentence_vec.size();
    for (size_t i = 0; i < n; ++i)
    {
        double sum = 0.0;
        for (size_t j = 0; j < n; ++j)
        {
            if (j != i)
                sum += GetWeight(i,j);
        }
        m_out_sum_map.insert(make_pair(i, sum));
    }
    
}

void SentenceRank::CalcSentenceScore(map<size_t, double> &score_map)
{
    score_map.clear();

    // initialize
    size_t n = m_sentence_vec.size();
    for (size_t id = 0; id < n; ++id)
        score_map.insert(make_pair(id, 1.0));

    // iterate
    for (size_t i = 0; i < m_max_iter_num; ++i)
    {
        double max_delta = 0;
        map<size_t, double> new_score_map; // current iteration score

        for (size_t id1 = 0; id1 < n; ++id1)
        {
            double new_score = 1 - m_d;
            double sum_weight = 0.0;
            for (size_t id2 = 0; id2 < n; ++id2)
            {
                if (id1 == id2 || m_out_sum_map[id2] < 1e-6)
                    continue;
                double weight = GetWeight(id2, id1);
                sum_weight += weight/m_out_sum_map.at(id2)*score_map[id2];
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

void SentenceRank::Clear()
{
    m_sentence_vec.clear();
    m_sentence_index.clear();
    m_weight_map.clear();
    m_out_sum_map.clear();
}
