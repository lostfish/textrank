# TextRank算法实现

## 编译
```
cmake ./
make
```
编译后会生成两个可执行文件: segment, textrank

segment分词并输出选择后的词, 输出为在输入文件最后追加一列, 用法：
```
 ./segment <input_file> <text_field> <out_file>
```

textrank抽取关键词或者关键句，输出为在输入文件最前面追加一列，用法：
```
 ./textrank <input_file> <choose_field> <method> <out_file>
 <method>: 1 -> keywords; 2 -> key sentences
 <choose_field>: if keywords, set as token field; if key sentences, set as content field
```

## 运行
```
sh run.sh
```
输入文件为./data/news.u8, 格式为两列：标题，正文

关键词文件为： ./data/news.word

关键句文件为： ./data/news.sent

## 参考

http://lostfish.github.io/posts/2016/09/17/textranksuan-fa-shi-xian.html
