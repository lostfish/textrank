#include<iostream>
#include<fstream>
#include<sstream>
#include<cstdlib>
#include "text_rank.h"
#include "sentence_rank.h"
#include "text_utils.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        cout << "Usage: " << argv[0] << " <input_file> <choose_field> <method> <out_file>" << endl;
        cout << "<method>: 1 -> keywords; 2 -> key sentences" << endl;
        cout << "<choose_field>: if keywords, set as token field; if key sentences, set as content field" << endl;
        return -1;
    }
    string input_file(argv[1]);
    int choose_field = atoi(argv[2]);
    int method = atoi(argv[3]);
    string out_file(argv[4]);

    if (method != 1 && method != 2)
    {
        cout << "<method>: 1 -> keywords; 2 -> key sentences" << endl;
        return -1;
    }

    ifstream fin(input_file.c_str());
    if (!fin.is_open())
    {
        cout << "fail to open file: " << input_file << endl;
        return -1;
    }
    ofstream fout(out_file.c_str());
    if (!fin.is_open())
    {
        cout << "fail to open file: " << out_file << endl;
        return -1;
    }


    int window_length = 3; 
    int max_iter_num = 100;
    double d = 0.85;
    double least_delta = 0.0001;
    size_t topN = 5;

    TextRank ranker(window_length, max_iter_num, d, least_delta);
    SentenceRank sent_ranker(window_length, max_iter_num, d, least_delta);

    if (method == 1)
    {
        string line;
        vector<string> fields;
        vector<string> token_vec;
        vector<pair<string, double> > keywords;
        vector<pair<string, double> > keywords2;
        while(getline(fin, line))
        {
            TextUtils::Split(line, "\t", fields);
            const string &token_str = fields[choose_field];
            TextUtils::Split(token_str, " ", token_vec);
            ranker.ExtractKeyword(token_vec, keywords, topN);
            ranker.ExtractHighTfWords(token_vec, keywords2, topN);
            for(size_t i = 0; i < keywords.size(); ++i)
            {
                if (i != 0)
                    fout << ' ';
                fout << keywords[i].first << '(' <<  keywords[i].second << ')';
            }
            fout << '\t';
            for(size_t i = 0; i < keywords2.size(); ++i)
            {
                if (i != 0)
                    fout << ' ';
                fout << keywords2[i].first << '(' <<  keywords2[i].second << ')';
            }
            fout << '\t' << line << endl;
        }
    }
    
    if (method == 2)
    {
        string line;
        vector<string> fields;
        vector<string> sent_vec;
        vector<string> bigram_vec;
        vector<pair<string, double> > key_sents;
        bool is_utf8 = true;
        while(getline(fin, line))
        {
            TextUtils::Split(line, "\t", fields);
            const string &content = fields[choose_field];

            TextUtils::SplitToSentence(content, g_seperator_str, sent_vec, is_utf8);

            map<string, vector<string> > sentence_token_map;
            for (size_t i = 0; i < sent_vec.size(); ++i)
            {
                //cout << "sent: " << sent_vec[i] << endl;
                TextUtils::ExtractNgram(sent_vec[i], 2, is_utf8, bigram_vec);
                if (bigram_vec.empty())
                    continue;
                sentence_token_map.insert(make_pair(sent_vec[i], bigram_vec));
            }
            sent_ranker.ExtractKeySentence(sentence_token_map, key_sents, topN);
            for(size_t i = 0; i < key_sents.size(); ++i)
            {
                if (i != 0)
                    fout << ' ';
                fout << key_sents[i].first << '(' <<  key_sents[i].second << ')';
            }
            fout << '\t' << line << endl;
        }
    }
    fin.close();
    fout.close();
    return 0;
}
