# Web Search Engine: Index Quantization

Haozhong Zheng
hz2675@nyu.edu
New York University

## ABSTRACT

在课程中，我们已经实现了一简单的反向索引建立以及查询的功能。在系统进行查询处理时，需要利用BM25等类似的算法计算查询与文档之间的相似度。为了加快速度，一种常见的解决方法是提前计算好影响分数并保存在索引文件中，以避免查询时的计算开销。然而浮点数的存储将会占据大量的空间，通用的数据压缩算法又难以提供理想的压缩率，因此必须对影响分数采取量化方法减小储存占据的空间。在本次期末项目中，我们阅读了近几年有关量化的论文，并尝试实现了传统的线性量化、对数量化，以及一种名叫adaptive float的新方法，并通过实验比较了他们各项指标。

**KEYWORDS:** Inverted Index, Linear Quantization, Logarithm Quantization, Adaptive Float

## 1 INTRODUCTION

### 1.1 Background

现代的互联网规模庞大，而且其容量依然在飞快的增长之中，这就给网络搜索引擎带来了非常大的挑战。例如全球最大的搜索引擎Google，根据相关统计，Google对超过十万亿的网页建立了索引。通常来说，用户希望最多等待数百毫秒就能在海量的网页中得到一个典型的查询结果，原始的信息提取技术是不可能做到这点的。
提高查询速度的方法有很多，其中最重要的几点是设计出先进的查询处理算法、高效的搜索引擎架构和索引结构。由于BM25等算法计算得出的影响分数与用户的查询无关，因此在所有改进的索引结构中，开发者都一定会存储提前计算的影响分数以避免查询时的计算开销。考虑到最新的搜索引擎往往会有万亿级别的网页索引，包含的词语甚至更多，存储未经任何压缩的浮点数将会占据天量的空间，并不可行。同时，对浮点数形式的影响分数使用通用的压缩方法如Gzip往往效果一般。如果开发者想要节省更多的空间，那就必须考虑各种浮点数的量化方法。
事实上，数据量化的方法非常多，早在网络搜索引擎出现以前，研究人员就已经投入大量精力研究开发各种解决方案了。量化在通信领域的应用最多，而网页词语的影响分数与通信类数据不同.因此许多量化方法不一定有很好的结果，这需要我们在本项目中进行对比实验。

### 1.2 Project Overview

本学期的网络搜索引擎课程一共包含四项作业。第一项作业为基于优先队列的网络爬虫，与本项目没什么关系。第二项为反向索引数据结构的建造，第三项为连接的与分离的查询处理。而本次期末项目的代码就基于以上两者的加以改进。
- Index Building: The program uses merging-subindexes algorithm instead of merging intermediate postings. Also, the inverted indexes and the corresponding lexicon terms are arranged in alphabetical orders. I divide the whole building process into two steps: The first step would parse the giving fulldocs-new.trec file, tokenize documents. Then it would convert tokens to to inverted index, build necessary data structures and write them to disk. The second step would perform n-way merge to build the final index. In this program, both document id and term frequency use var-byte compression.
- Query Processing: The program implements basic "Document-At-A-Time"conjunctive query and "Term-At-A-Time" disjunctive query processing. Also it supports complex query, which would process disjunctive query for multiple conjunctive terms or single terms. For example, A complex query string "microsoft, apple || band" would transform to "(microsoft && apple) || (band)" . This is implemented by some kind of polymorphism design.

但是，之前作业中的反向索引储存的是词语在每篇文档中出现的频率，并使用var-byte压缩方式。要想将存储的频率数据转换为提前计算的影响分数，再进一步应用量化方法，这需要我们的代码拥有更好的泛用性、可扩展性。为此，在本次项目中我们大幅度重构了原有的代码，将冗余的逻辑分离成不同的类，并大量使用C++模板编程使得派生类能继承共用的逻辑。这样一来与反向索引相关的各种数据结构都能更加方便的扩展功能，例如量化或压缩。

## 2 RELATED WORKS

### 2.1 Search Engine

信息提取技术被广泛的应用于计算机领域的各种应用中，搜索引擎是其中之一。现在的国际互联网搜索引擎市场的垄断状况十分严重。排在前位的只有Google, Microsoft Bing, Yahoo, Baidu, Yandex and DuckDuckGo，这些商业搜索引擎都是闭源的。在开源的搜索引擎中，最成熟的是Apache Lucene，它是由Apache Software Foundation负责支持的Java搜索引擎library。除此之外，还有一些开源搜索引擎例如PISA系统(Performant Indexes and Search for Academia), which is an open source library implementing text indexing and search, primarily targeted for use in an academic setting. [1]
一个完整的搜索引擎系统至少要拥有以下几种功能：
- Documents Parsing: Tokenize documents to get terms and build postings, where each term is assigned a unique document ID, and each document consists of a list of postings.
- Indexing: Once the parsing phase is complete, the postings containing a collection can be used to build an inverted index, which is a collection of terms,
each a containing a list of document IDs.
- Scoring: BM25 is a simple yet effective ranking function for processing bag-of-words queries, so currently it's widely used in many systems.
- Index Compression: 这不仅包括了对document ID或是频率的压缩，还包括了对影响分数的压缩与量化。
- Search: Apart from primitive index traversal strategy, the system should also support dynamic pruning algorithms to improve query performance.

### 2.2 Data Compression

在过去的二十年里，计算机与通信技术在快速地发生变革，互联网时代的信息爆炸更是体现出了数据压缩技术的重要性。通常数据压缩技术可以分为无损压缩与有损压缩两种。
无损压缩，顾名思义，压缩的过程不存在信息损失。它被应用于不允许原数据与解压数据有任何区别的场景，在不给定用户需求的情况下，当然不应该对原数据做任何改变，因此通用压缩方式都采用的是无损压缩。
在无损压缩算法中，最著名的是1977年与1978年发布的LZ77与LZ78 (Lempel–Ziv) 算法。而在LZ77算法的基础上，又衍生出了许多压缩算法，被称为LZ77家族。这其中包括Lempel–Ziv–Welch (LZW) 算法，Lempel–Ziv–Markov chain (LZMA) 算法，Lempel–Ziv–Oberhumer (LZO) 算法以及发表时间较新的LZ4算法。而使用最多的gzip压缩文件格式采用的DEFLATE算法是LZ77算法与哈夫曼编码的结合。除此之外，还有采取了Burrows–Wheeler 算法的bzip2压缩文件格式也有较好的效果。HDF5高性能数据library还可以使用诸如bitshuffle一类的算法进行预处理。[2]
事实上，许多应用能够接受一定程度的信息损失，并不需要完全准确的复原数据，这种时候就会采用有损压缩技术。显然，针对特定的应用而设计的有损压缩算法会比通用的无损压缩算法有着更高的压缩率。
有损压缩的常见应用集中在音频、视频或图像等多媒体数据上，这与本项目所处理的影响分数不同。不过，为了对大量科学研究中的浮点数进行压缩，现在也出现了一些通用有损压缩算法，并且得到了非常好的效果。例如ZFP压缩器，能对多维度的整数或浮点数进行压缩。[3]近几年出现的SZ压缩器实现了a hybrid Lorenzo prediction method called mean-integrated Lorenzo prediction 和 a linear regression method that can obtain much higher prediction accuracy. It currently has state-of-the-art performance. [4][5][6] 总的来说，数据压缩是一个很宽广的研究领域，不少论文[7]都对现在最先进的各种数据压缩技术进行了综述与对比。

### 2.3 Quantization

量化是有损压缩技术的一种，而且是最简单、最常见的思想。在许多有损压缩应用中，需要将原数据集的一个或多个值表示为少量码字之一。数据集中的取值个数通常远大于可用于表示它们的码字数目。用一个元素数目很小的值集来表示一个非常大（很可能是无限）的值集，这一过程就被称为量化。
量化也可以分为标量量化与矢量量化两种，当量化器的输出值一一对应输入值时，叫做标量量化，而当多个输入值被量化为一个输出值时，叫做矢量量化。本次项目中实现的线性量化与对数量化就是标量量化中的方法；还有根据输入的统计指标来进行调整的自适应量化方法，指标包括均值、方差和概率密度函数 (PDF)；已知数据的概率模型，最佳的非均匀量化方法是Lloyd-Max算法。近年来，随着深度神经网络的规模变大，对神经网络的浮点数权重进行量化的论文逐渐占据了量化领域的主流，因此我们也参考了这些与深度学习有关的量化方法。
矢量量化的思想是对输入值进行分组，然后将每组看成一个整体进行量化，得到相应码字。这样一来，矢量量化的主要任务其实与聚类有关，因此相关的算法也都含有聚类的内容。Classical vector quantization methods include Linde-Buzo-Gray (LBG) algorithm (which is an extent of Lloyd-Max algorithm), pair-wise nearest neighbor (PNN) algorithm, the simulated annealing (SA) algorithm, and the fuzzy c-means clustering analysis (FCM) algorithm. 在这篇论文[8]中就比较了几种算法的表现。

## 3 METHODS

### 3.1 Linear Quantization

如果说量化是有损压缩技术中最简单和常见的思想，那么线性量化（或均匀量化）就是量化技术中最简单和常见的方法。该方法假定数据在一定的区间内均匀分布，那么只要给定一个分隔数目，就能分隔为大小相等的子区间。将输入的数值映射到某个子区间的索引就完成了线性量化。
我们假设区间在[-M, M]，而量化的比特位数是B，那么区间将被分隔为2^B个，量化公式为：

反量化公式为：


### 3.2 Logarithm Quantization

通过分析本项目中数据集的BM25分数分布，很容易发现它们并不是均匀分布的，因此线性量化不会取得很好的效果。在深度神经网络中，权重参数常常集中在0附近，并且规模越大的网络越符合这个规律。这其实与非均匀分布的BM25分数有一定的相似之处，因此我们可以从尝试深度神经网络的相关量化方法。有一种名叫QuanSeries [9]的方法就使用了改进的对数量化，应用在权重集中在0附近的神经网络有很好的效果。
对数量化神经网络的传统方法只是简单地对对数取整，如公式所示。

这样的方法具有很多缺点，比如：4bits or larger bit-width quantization brings little to no accuracy improvement than 3 bits quantization. Manually fine tuning the FSR (full scale range) parameter is cumbersome.


### 3.3 Adaptive Float

```python

```

## 4 IMPLEMENTATION

### 4.1 C++ Template Programming

### 4.2 Memory Counter

### 4.3 Bits Vector

### 4.4 Linear Quantization

### 4.5 Logarithm Quantization

### 4.6 Adaptive Float

## 5 EXPERIMENT RESULTS

## 6 FUTURE WORKS

## REFERENCES

[1] Mallia, A., Siedlaczek, M., Mackenzie, J., & Suel, T. (2019). PISA: Performant indexes and search for academia. Proceedings of the Open-Source IR Replicability Challenge.
[2] Masui, K., Amiri, M., Connor, L., Deng, M., Fandino, M., Höfer, C., ... & Vanderlinde, K. (2015). A compression scheme for radio data in high performance computing. Astronomy and Computing, 12, 181-190.
[3] Lindstrom, P. (2014). Fixed-rate compressed floating-point arrays. IEEE transactions on visualization and computer graphics, 20(12), 2674-2683.
[4] Di, S., & Cappello, F. (2016, May). Fast error-bounded lossy HPC data compression with SZ. In 2016 ieee international parallel and distributed processing symposium (ipdps) (pp. 730-739). IEEE.
[5] Tao, D., Di, S., Chen, Z., & Cappello, F. (2017, May). Significantly improving lossy compression for scientific data sets based on multidimensional prediction and error-controlled quantization. In 2017 IEEE International Parallel and Distributed Processing Symposium (IPDPS) (pp. 1129-1139). IEEE.
[6] Liang, X., Di, S., Tao, D., Li, S., Li, S., Guo, H., ... & Cappello, F. (2018, December). Error-controlled lossy compression optimized for high compression ratios of scientific datasets. In 2018 IEEE International Conference on Big Data (Big Data) (pp. 438-447). IEEE.
[7] Duwe, K., Lüttgau, J., Mania, G., Squar, J., Fuchs, A., Kuhn, M., ... & Ludwig, T. (2020). State of the Art and Future Trends in Data Reduction for High-Performance Computing. Supercomputing Frontiers and Innovations, 7(1), 4-36.
[8] Huang, C. M., & Harris, R. W. (1993). A comparison of several vector quantization codebook generation approaches. IEEE Transactions on Image Processing, 2(1), 108-112.
[9] Cai, J., Takemoto, M., & Nakajo, H. (2018, December). A deep look into logarithmic quantization of model parameters in neural networks. In Proceedings of the 10th International Conference on Advances in Information Technology (pp. 1-8).
[10] Tambe, T., Yang, E. Y., Wan, Z., Deng, Y., Reddi, V. J., Rush, A., ... & Wei, G. Y. (2019). Adaptivfloat: A floating-point based data type for resilient deep learning inference. arXiv preprint arXiv:1909.13271.