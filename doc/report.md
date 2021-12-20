# Web Search Engine: Index Quantization

Haozhong Zheng
hz2675@nyu.edu
New York University

## ABSTRACT

在课程中，我们已经实现了简单的反向索引建立以及查询的功能。在系统进行查询处理时，需要利用BM25等类似的算法计算影响分数。为了加快速度，一种常见的解决方法是提前计算好影响分数并保存在索引文件中，以避免查询时的计算开销。然而浮点数的存储将会占据大量的空间，通用的数据压缩算法又难以提供理想的压缩率，因此必须对影响分数采取量化方法减小储存占据的空间。在本次期末项目中，我们阅读了近几年有关量化的论文，并尝试实现了传统的线性量化、对数量化，以及一种名叫adaptive float的新方法，并通过实验比较了他们各项指标。

**KEYWORDS:** Inverted Index, Linear Quantization, Logarithmic Quantization, Adaptive Float


## 1 INTRODUCTION

### 1.1 Background

现代的互联网规模庞大，而且其容量依然在飞快的增长之中，这就给网络搜索引擎带来了非常大的挑战。例如全球最大的搜索引擎Google，根据相关统计，Google对超过十万亿的网页建立了索引。通常来说，用户希望最多等待数百毫秒就能在海量的网页中得到一个典型的查询结果，原始的信息提取技术是不可能做到这点的。
提高查询速度的方法有很多，其中最重要的几点是设计出先进的查询处理算法、高效的搜索引擎架构和索引结构。由于BM25等算法计算得出的影响分数与用户的查询无关，因此在所有改进的索引结构中，开发者都一定会存储提前计算的影响分数以避免查询时的计算开销。考虑到最新的搜索引擎往往会有万亿级别的网页索引，包含的词语甚至更多，存储未经任何压缩的浮点数将会占据天量的空间，并不可行。同时，对浮点数形式的影响分数使用通用的压缩方法如Gzip往往效果一般。如果开发者想要节省更多的空间，那就必须考虑各种浮点数的量化方法。
事实上，数据量化的方法非常多，早在网络搜索引擎出现以前，研究人员就已经投入大量精力研究开发各种解决方案了。量化在通信领域的应用最多，而网页词语的影响分数与通信类数据不同.因此许多量化方法不一定有很好的结果，这需要我们在本项目中进行对比实验。

### 1.2 Project Overview

本学期的网络搜索引擎课程一共包含四项作业。第一项作业为基于优先队列的网络爬虫，与本项目没什么关系。第二项为反向索引数据结构的建造，第三项为连接的与分离的查询处理。而本次期末项目的代码就基于以上两者的加以改进。
- Index Building: The program uses merging-subindexes algorithm instead of merging intermediate postings. Also, the inverted indexes and the corresponding lexicon terms are arranged in alphabetical orders. I divide the whole building process into two steps: The first step would parse and tokenize the documents. Then it would convert tokens to to inverted index, build necessary data structures and write them to disk. The second step would perform n-way merge to build the final index. In this program, both document id and term frequency use var-byte compression.
- Query Processing: The program implements basic "Document-At-A-Time" conjunctive query and "Term-At-A-Time" disjunctive query. Also it supports complex query, which would process disjunctive query for multiple conjunctive terms or single term. For example, A complex query string "microsoft, apple || band" means "(microsoft && apple) || (band)" . This is implemented by some kind of polymorphism design.

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
我们假设区间在`[min, max]`，而量化的比特位数是`B`，那么区间将被分隔为`2^B`个，量化公式为：
<eq1>
反量化公式为：
<eq2>

### 3.2 Logarithmic Quantization

通过分析本项目中数据集的BM25分数分布，很容易发现它们并不是均匀分布的，因此线性量化不会取得很好的效果。在深度神经网络中，权重参数常常集中在0附近，并且规模越大的网络越符合这个规律。这其实与非均匀分布的BM25分数有一定的相似之处，因此我们可以从尝试深度神经网络的相关量化方法。有一种名叫QuanSeries [9]的方法就使用了改进的对数量化，应用在权重集中在0附近的神经网络有很好的效果。
传统的对数量化方法只是简单地对对数取整，这样的方法具有很多缺点。 To improve them, the paper extends the exponents from integer domain to the real number field thus quantized weights now have a higher density.
Firstly, it generates an arithmetic progression via equation (3) called QuanSeries, this would be kept as a lookup table. Parameter R is a positive real number. In practice, R could be tuned based on the distribution of values. Secondly, the quantization step would use argmin method to find the indexes of nearest values in lookup table, and these indexes are the quantized results.

<eq3>
<eq4>

Given that quantized results are the indexes of lookup table, dequantization is to simply get the exponents from table and compute the power of 2.

<eq5>

<figure1>

### 3.3 Adaptive Float

Adpaptive Float同样是一种面向深度神经网络而设计的浮点数量化方法，由于它具有一定的自适应能力，我们认为该方法很有可能对于影响分数的量化也有不错的效果。
The Adaptive Float number representation generally follows the IEEE 754 Standard floating-point format that includes a sign bit, exponent bit, and mantissa bit fields. At very low bit compression, the exponent representation becomes tricky. Thus, similar to integer quantization that uses a quantization scale (or step), we introduce a bias value, `exp_bias`, to dynamically shift the range of exponent values.
下面的代码就描述了Adaptive Float的量化过程。它定义了一个数总共所占的位数`n`，和其中指数位所占位数`e`。首先，它求出原数据的符号位`arr_sign`和绝对值`arr_abs`。然后，它根据绝对值的最大值求出`exp_bias`与`exp_max`，以及对应的`value_min`与`value_max`，这样一来就可以修剪数据中过大或过小的数值了。一个典型的`exp_bias`应该为负数，而将数据中的每个指数减去`exp_bias`后，我们就会得到指数全都为正数，这就避免了处理极小位数的负指数问题，简化了算法。

```python
# Get Mantissa bits
m = n - e - 1
# Obtain sign and abs
arr_sign = sign(arr)
arr = abs(arr)
# Determine exp_bias and range
# Find normalized exp_max for max(arr) such that
# 2**exp_max < max(arr) < 2**exp_max+1
exp_bias = exp_max - (2**n_exp - 1)
value_min = 2**exp_bias*(1 + 2**(-m))
value_max = (2**exp_max)*(2 - 2**(-m))
# Handle unrepresentable values
arr[arr < 0.5*value_min] = 0
arr[(arr > 0.5*value_min) & (arr < value_min)] = value_min
arr[arr > value_max] = value_max
# get mantissa and exponent
mant, exp = frexp(arr)
# Quantize arr
q_exp = exp - exp_bias
q_mant = round(mant * (2**m-1))
```


## 4 IMPLEMENTATION

### 4.1 C++ Template Programming

在1.2小节中，我们已经提及到我们在本次项目中大幅度重构了原有的代码，以增加程序的可扩展性。举例来说，我们的代码中主要包含以下的类：
- Lexicon: Map a term to meta data of the index list
- Index: Includes the block-wise compressed data, and the meta data of blocks.
- IndexForwardIter: Get items from index list, uncompress a block to cache once at a time.
- IndexBackInserter: Append items to index list, compress a block from cache when it's full.
- InputBuffer/OutputBuffer: Sequentially read/write data from/to disk.
很显然，当需要保存或读取不同类型的数据时，就需要通过C++的模板来派生出不同的类，而基类中共用的逻辑也需要调用派生类各自的方法，这就涉及到了多态。一般提到多态时，我们指的都是运行时多态，这需要代码用virtual关键字来指明，并且在运行时通过虚函数指针来调用不同的派生类方法，会有一定的性能损失。因此，在本次项目中我们尝试使用了一种名叫The Curiously Recurring Template Pattern (CRTP) 的C++编程方式，来实现静态多态。

```c++
template <typename T>
class Base {
public:
    friend class Derived;
    void doSomething() {
        T& derived = static_cast<T&>(*this);
        // use derived...
        derived.toBeCalledFromBase();
    }
};

class Derived :public Base<Derived> {
private:
    int x;
public:
    void toBeCalledFromBase() {
        x = ...;
    }
};
```

### 4.2 Memory Counter

在反向索引的建立过程中，input buffer 和 output buffer 应当拥有各自的内存上限。在工程实践中，开发者们常用的办法是限制整个进程的内存上限，或是仅仅限制一个容器内的数据个数，因为计算特定对象所占据的内存空间是非常困难的。实现这一点需要绑架默认的内存分配函数，这样才能准确地得出为某些对象分配的内存空间。在本次项目中，我们仅仅是出于探索的目的尝试计算内存，工程实践中不可能使用这种方法。
C++ STL的容器类使用allocator分配内存，但是默认的allocator是无状态的，这意味着我们无法用变量进行统计。C++17 introduced stateful std::pmr::polymorphic_allocator, which is an allocator that exhibits different allocation behavior depending upon the std::pmr::memory_resource from which it is constructed. By overriding the allocation methods of std::pmr::memory_resource, we can easily sum up the memory consumption within specific containers.
另外，我们还使用了第三方的内存池mimalloc来加快程序运行速度。mimalloc is a free and open-source compact general-purpose memory allocator developed by Microsoft with focus on performance characteristics. The library works as a drop-in replacement for malloc of the C standard library and requires no additional code changes.

### 4.3 Bits Vector

当浮点数量化为一定宽度的比特位后，为了节省空间，比特之间应该连续的存储，而不是以字节为单位分隔。无论是C++的STL还是BOOST都有相关的比特数组容器可用，但我们依然决定自己实现一个简单的bits vector。由于我们的索引数据以块为单位存储，因此我们的设计是一个块之内的比特连续的存储，而块和块之间以字节分隔。图2是存储方式的示意图。而图3展示了从比特之中提取一个数的过程。以此类推，向bits vector中增加比特的过程就是图3的反向过程。

<figure2>
<figure3>

### 4.4 Quantization

如3.1小节所讲，在实现线性量化时，我们先将数值映射为`[0, 1]`范围内的实数，然后再映射为`[0, 2^B-1]`的整数。这样一来就出现了一个问题：我们应该将实数乘以`2^B`，还是`2^B-1`？前者需要处理溢出比特宽度的整数`2^B`，而后者会损失一定的精度。在实验过程中，我们发现可以采用一种介于前者与后者之间的方法，例如设定`eps=0.501`，乘以`2^B-eps`就避免了除以溢出值的问题。

<eq5>
<eq6>

3.2小节所描述的方法来自论文，仅仅适用于神经网络权重的量化。它的指数取值范围只是`[0, -R]`，并且无法处理负浮点数。

<figure4>

```python
eps = 0.501
# mant: [0.5, 1.0) -> [0.0, 1.0)
mant = 2*mant-1
if sign > 0:
    # q_mant: [1, 2**m-1]
    q_mant = round(mant * (2**m-1-eps))
elif sign < 0:
    # q_mant: [2**m, 2**(m-1)-1]
    q_mant = round(mant * (2**m-eps)) + 2**m
```


## 5 EXPERIMENT RESULTS

### 5.1 Sample Visualization

### 5.2 Benchmark

## 6 FUTURE WORKS

量化以及压缩领域包含了太多的方法，而我们在本次期末项目中仅仅是实现了3种比较基础的量化方法，拥有很多的不足之处。比如说，实现的3种方法全部都是标量量化，而没有一种是矢量量化。根据许多文献中的实验结果，矢量量化所耗费的时间长、且算法难以收敛，但一般都能获得比标量量化更好的结果。似乎近年来没人采用这些矢量量化算法与搜索引擎的影响分数相结合并进行实验，因此值得我们尝试。并且，在量化之后还可以考虑使用上文中所描述的一些通用压缩方式进一步的压缩。
更重要的是，由于时间不足，本次项目并没有真正实现不同量化方式的查询处理。然而，查询结果的质量对比才是搜索引擎中最需要关心的事情。在未来进一步的工作中，这些都需要充分考虑到。


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