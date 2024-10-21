## Web Search Engine Assignment3 - Query Processing

**Haozhong Zheng, hz2675@nyu.edu**



### Summary

#### Programming Language

My program consists of both C++ and python code.

- The C++ part has all data structures needed. These structures have major functions used for Index Building and Query Processing tasks. C++ part provides python interface, it should be compiled to dynamic link library for python to call.
- The python part is where the program executes. It basically parse the command line arguments, load C++ DLL, then run certain procedures to call C++ and get the result. Using python is because it's easier to implement trivial things that aren't strongly related to Index Building or Query Processing.
- For Index Building, the c++ part includes DocTable, Lexicon, Inverted Index, Postings and Input or Output Buffers; the python executable is `IndexBuilder.py`.
- For Query Processing, the c++ part includes everything from Index Building, and it implements LRUCache, BM25 score computing, disjunctive or conjunctive query processors; the python executable is `Query.py`.



#### Features and Algorithms for Index Building

The program uses merging-subindexes algorithm instead of merging intermediate postings. Also, the inverted indexes and the corresponding lexicon terms are arranged in alphabetical orders (more precisely, byte order, because there're utf-8 characters).

Therefore, I divide the whole building process into two steps:

- The first step would parse the giving `fulldocs-new.trec` file, tokenize documents. Then C++ part would build doc table, convert tokens to postings, then to inverted index, build lexicon and write file to disk when index buffer reaches memory limit.

- The second step would perform n-way merge to build the final index which should be I/O efficient if the buffer size and N is properly set.

The program can conveniently set the number of n-way merge, size of input and output buffer by command line. It supports utf-8 text file format or compressed binary format by changing compiler flag.



#### Features and Algorithms for Query Processing

The program implements basic conjunctive and disjunctive query processing, and also complex query, which would process disjunctive query for multiple conjunctive terms or single terms.

For example:

- A regular query string `"#microsoft#, apple."` would transform to conjunctive query `"microsoft && apple"`
- Explicitly enters `"||"` to indicate disjunctive query like `"microsoft || apple"`.
- A complex query string `"#microsoft#, apple. || band"` would transform to `"(microsoft && apple) || (band)"`.  This is implemented by some polymorphism design.

The python part would get the query result and generate document snippets, print result in specific format.

To improve query performance, it uses a LRU Cache to keep index in memory. When a query is requested, it would get index from memory in fast speed, or load from file and put into memory.

The program can conveniently set size of LRU Cache, the number of query results and the length of snippet by command line. It also supports reading or writing terms in LRU Cache from/to file.



### Environment and Compile

- `python`: python3, with module `nltk`

- `C++`: I use msvc on windows, but other compilers on other platforms should work well too. The compiler should build dynamic link library.

  If preprocessor defines `TEXT_FILE`, then the file is utf-8 text format, otherwise it's compressed binary format.
  
- For Index Building, should compile all files in `Index/` folder. For Query Processing, compile all files in both `Index/` and `Query/` folders.



### Command Line Arguments for Index Building

Execute the Index Building program by command `python IndexBuilder.py` with arguments. As I mentioned above, the program is divided into two steps, so it should execute two times with different arguments.

Arguments needed for both steps:

- -o "path": output directory
- --dll "filename": c++ dll filename
- --ibuf \<int>: total input buffer size in MB
- --obuf \<int>: total output buffer size in MB
- --ext "txt": extension of output files, default is `bin` (only affect filename, not actual format)

Arguments for step1:

- --step1
- -i "filename": input `.trec` filename

Arguments for step2:

- --step2
- -m \<int>: number of files from the output of step1
- --way \<int>: n-way merge



### Command Line Arguments for Query Processing

Execute the Query Processing program by command `python Query.py` with arguments.

- --buf \<int>: index LRU cache size in MB
- -i "filename": documents dataset `.trec` filename
- --dll "filename": c++ dll filename
- --index "filename": index filename from Index Building
- --lexicon "filename": lexicon filename from Index Building
- --doctable "filename": doctable filename from Index Building
- --load-cache "filename": if this argument is specified, load LRU Cache from file
- --dump-cache "filename": if this argument is specified, dump LRU Cache on file
- -k \<int>: top K result
- -s \<int>: snippet length for each query word



### Files and Data Structures

#### Index Building

**DocTable**

- `Index/DocTable.h`, `Index/DocTable.cpp`
- A structure that, given a docID, returns the URL of the page and the size (number of terms) of the page (the second thing is needed in the cosine measure).
- Data structures and file format:

```c++
struct DocTableItem {
    uint64_t start_offset; /* start offset of doc in file */
    uint32_t len; /* # of terms */
    std::string url;
};
class DocTable {
    float avg_len; /* average number of terms in each document */
    std::vector<DocTableItem> table;
    ......
};
/*
    text format:
    <avg_len> (\n)
    <start offset> (space) <# of terms> (space) <url> (\n)
    binary format:
    <4-avg_len>
    <8-start offset> <4-# of terms> <str-url> <1-\0>
*/
```

- doc_id is the order the pages are parsed
- Includes methods for file reading and writing with text format or binary format

**Lexicon**

- `Index/Lexicon.h`, `Index/Lexicon.cpp`
- a structure that given a word, returns the start of the corresponding inverted list in the index, the end of the list, and the number of docs containing the word (the last thing is needed to evaluate the cosine measure).
- Data structures and file format:

```c++
struct LexiconValue {
    uint64_t start_offset; /* start offset in index file of a term */
    uint64_t end_offset; /* end offset in index file of a term */
    uint32_t n_docs; /* # of docs containing the term */
};
class Lexicon {
    std::map<std::string, LexiconValue> terms;
    ......
};
/*
    text format:
    <term> (space) <start offset> (space) <end offset> (space) <n_docs> (\n)
    binary format:
    <str-term> <1-\0> <8-start offset> <8-end offset> <4-n_docs>
*/
```

- Use ordered map to maintain alphabetical order of terms.

- Includes methods for file reading and writing with text format or binary format

**PostingsBuffer**

- `Index/Lexicon.h`, `Index/Lexicon.cpp`
- An intermediate data structure kept in memory that used in first step. It estimates the memory cost of its data, when buffer is full, postings should be convert to sub-indexes.
- Data structures:

```c++
struct Posting {
    uint32_t doc_id;
    uint32_t frequency;
};
class PostingsBuffer {
    size_t capacity;
    size_t byte_size;
    std::map<std::string, std::vector<Posting>> terms;
    ......
};
```

- Use ordered map to maintain alphabetical order of terms.

**CompressedBytes**

- `Index/hepler.h`, `Index/helper.cpp`
- A structure for var-byte compression.
- Data structures: `std::vector<uint8_t>`
- Includes `compress`, `decompress`, `difference` and `undifference` methods. Also the file have some global parameters and other utility methods.

**IndexTerm**

- `Index/IndexTerm.h`, `Index/IndexTerm.cpp`
- A structure for a single inverted index list. Postings are kept in **CompressedBytes form and doc_ids are taken differences**.
- Data structures and file formats:

```c++
struct BlockMetaData {
    uint32_t last_did; /* last doc_id of the block */
    uint16_t did_bsize; /* byte size of compressed doc_id */
    uint16_t freq_bsize; /* byte size of compressed frequency */
};
class IndexTerm {
    CompressedBytes b;
    /* associated to CompressedBytes */
    std::vector<BlockMetaData> blocks_info;
    /* a block of uncompressed, undifferenced doc_id and frequency cache */
    vector_u32 did_cache;
    vector_u32 freq_cache;
    LexiconIter info;
    ......
};
/*
    text format: uncompressed ascii numbers
    a block: <last_did> (space) <# of postings in this block> (space) [<doc_id> (space) ....] [<frequency> (space) ....] (\n)
    binary format:
    a block: <4-last_did> <2-did_size> <2-freq_size> <did_size-CompressedBytes> <freq_size-CompressedBytes>
*/
```

- Compressed postings are not split by blocks, they are kept continuously in one single array. So the class uses meta data to identify blocks.
- An iterator points to the corresponding Lexicon item, so it can get the needed info.
- `get_next` method, `append` method and `get_nextGEQ` method. Used for transferring (merging) postings of IndexTerm from input buffer to IndexTerm from output buffer.
- This data structure includes a block of uncompressed postings cache used for `get_next` and `append` method. During the transfer, it maintains the cache by load or unload from compressed bytes.
- Includes methods for file reading and writing with text format or binary format.

**IndexBuffer**

- `Index/IndexBuffer.h`, `Index/IndexBuffer.cpp`
- A structure that has a linked list of IndexTerm which is easier to erase or insert than array. It has capacity to check if the buffer is full.
- Data structures:

```c++
class IndexBuffer {
    size_t capacity;
    size_t byte_size;
    std::list<IndexTerm> ilist;
    ......
};
```

- Each IndexBuffer is associated to a Lexicon object. For simplicity, memory cost of Lexicon is not counted. To save memory, Lexicon item will be removed or added along with IndexTerm list item.
- While inserting postings, IndexTerm object would compute the estimated memory size, then add into IndexBuffer.
- Includes methods used for merging, data transferring.



#### Query Processing

**IndexLRUCache**

- `Query/IndexLRUCache.h`, `Query/IndexLRUCache.cpp`
- A structure that is derived from class IndexBuffer, it added an hash map to get the index by string.
- Data structures:

```c++
class IndexLRUCache : public IndexBuffer {
    std::unordered_map<std::string, IndexTermIter> map;
    ......
};
```

- If an index list is too long (e.g. a few millions postings that are a dozen megabytes), it may cost too much memory to keep the whole list. In this case LRU Cache would limit number of blocks in memory (maybe a few thousands blocks.)
- Includes typical methods for LRU:`get_index` and `put_index`. Also there're some wrapper methods from IndexTerm which would automatically handle reading blocks and count memory cost.
- Supports reading or writing terms in LRU Cache from/to file.

**QueryProcessor**

- `Query/QueryProcessor.h`, `Query/QueryProcessor.cpp`
- As described above, the program supports complex query, so there should be polymorphism with abstract class.
- Class design:

```c++
class Query {
    static float BM25_score(Posting p, IndexTerm& index);
    /* get the next document score from QueryProcessor */
    virtual QueryScore get_next() = 0;
    /* get the final topK query result */
    virtual cQueryResultArray* query_result(uint32_t topK) = 0;
    virtual void reset_index_state() = 0;
    virtual ~Query() {}
    ....
};
```

- 3 kinds of QueryProcessor
  
  - SingleQueryProcessor: query for a single term.
  
  - ANDQueryProcessor: conjunctive query, has an array of index pointers.
  
    `std::vector<IndexTermIter>`
  
    Sort index pointers in ascending order by number of postings in index.
  
  - ORQueryProcessor: disjunctive query, has an array of pointers of Query abstract class that has polymorphism feature.
  
    `std::vector<Query*>`
- To get top K query result, it uses a priority queue (min heap) to keep K query scores.
- Includes a query result data structure to communicate between python and C++.

```c++
struct QueryScore {
    uint32_t doc_id;
    float score; /* BM25 score of the document */
    static bool less(const QueryScore& a, const QueryScore& b);
    static bool greater(const QueryScore& a, const QueryScore& b);
};

using MinScoreHeap = std::priority_queue<QueryScore, std::vector<QueryScore>, decltype(&QueryScore::greater)>;

struct cQueryResultArray {
    struct cQueryResult {
        uint32_t doc_id;
        float score;
        char* url;
        uint64_t offset; /* start offset of doc in file */
        uint32_t len;
    };
    uint32_t n;
    cQueryResult* arr;
    cQueryResultArray(uint32_t a) :n(a) { arr = new cQueryResult[n]; }
};

```



#### Others

- `IndexBuilder.py`: Run the Index Building program.
- `Index/PyInterface.h`, `Index/PyInterface.cpp`: Major functions exposed to python code `IndexBuilder.py`.
- `Query.py`: Run the Query Processing program
- `Quyer/PyInterface.h`, `Quyer/PyInterface.cpp`: Major functions exposed to python code `Query.py`.



### Procedures Description for Index Building

**Step 1**

1. Get command line argument, set the size of input PostingsBuffer and output IndexBuffer.
2. Read lines of `.trec` file, parse with tags like `<DOC>`, `<TEXT>` and `</TEXT>`, get text of each document.
3. Use regular expression and `nltk` module to tokenize text. Pass info of document to C++ to build DocTable. Then pass these tokens to C++ to build postings of one document. Finally add these postings to PostingsBuffer.
4. If PostingsBuffer is full, transfer postings to output IndexBuffer and Lexicon. After the transfer, write all data in IndexBuffer to file.

**Step 2**

1. Get command line argument, compute the merge loop iterations based on number of files from the output of step1 and n-way.
2. For each merging run, input filenames and output filename follow some format, get these filenames. Call `merge` method in C++ for a single merge procedure, merge subindexes and sublexicon from a list of input buffers to output buffer.
3. In `merge` method, each iteration it gets the front IndexTerm in the list from all input buffers. Compare these IndexTerms to get the one with most prior alphabetical order, transfer this index to output buffer.
4. After transfer, erase this front IndexTerm. If this Input buffer now is empty, read from file to fill the buffer. Then continue iteration until everything is transferred.

**Details**

- The program guarantees alphabetical order, and doc_ids in files with prior order are lower than in other files. So the merge would be simple: just get the front index from input buffer and push it to the back of output buffer.



### Procedures Description for Query Processing

**User input and result output**

1. A main loop that repeatedly receives user input as query string.

2. Split the query string by disjunctive indicator `"||"`, get a list of sub query strings.

3. For each sub query string, tokenize to get a list of terms. Get these indexes from LRU Cache.

4. Construct QueryProcessors on sub query terms.

   4.1. If there's only one term, construct a SingleQueryProcessor.

   4.2. If there're more than one terms, construct an ANDQueryProcessor.

   4.3. If at least one of the terms are not found in Lexicon, return NULL.

5. Get the root QueryProcessor to be executed.

   5.1. If there's only one valid processor, execute it.

   5.2. If there're more than one valid processors, construct and execute an ORQueryProcessor.

   5.3. If there's no valid processor, print error.

6. For each document in query result, print doc_id, BM25 score, url and generate snippets.

   6.1. Read and parse the corresponding document contents.

   6.2. Use regular expression to match the query terms, get the intervals of these terms in contents string.

   6.3. Expand intervals to create snippets.

   6.4. Merge intervals.

   6.5 Join the corresponding strings by intervals to get final snippets.

**Query processing**

For LRU Cache `get_index` and `put_index` methods:

- If the requested index is currently kept in memory, get it directly. Move index to front list of LRU Cache.
- If not, read from file, put it to front list of LRU Cache.
- If cache is full, repeatedly erase the index at back list until cache remaining capacity is enough.

For Query Processor:

- Conjunctive query repeatedly calls `get_nextGEQ`  method until all terms get the same doc_id.
- Disjunctive query uses a large array with the length of number of documents which map doc_id to score.

<div style="page-break-after: always;"></div>

### Limitations

1. Estimate the memory cost of an object is difficult, I count the size in primitive way so it's inaccurate.
2. `nltk.tokenize` doesn't support other languages, and numbers are hard to decide how to tokenize. Since python has many great language processing modules, this problem can be improved further.
3. No multiprocessing during merging to improve the performance. This is also easy to solve because multiprocessing in python is easy to implement, but I don't have enough time.
4. No index sharding, essential-list or other techniques to improve query performance. Disjunctive query for terms which have very long index lists has to iterate the whole list and it can be very slow.
5. No advanced snippets generating algorithm. So the document content snippets are primitive.



### Index Building Result

- Dataset: https://microsoft.github.io/msmarco/TREC-Deep-Learning-2020

  [fulldocs-new.trec](https://msmarco.blob.core.windows.net/msmarcoranking/msmarco-docs.trec.gz);	size: 21.4GB;	number of documents: 3,213,835
  
- Step1:

  - input buffer: 2600MB;	output buffer: 500MB
  - Total number of output files: 6
  - **Elapsed time:** 72min 21.0sec

- Step2:

  - input buffer: 3200MB;	output buffer: 800MB
  - **6-way merge:** 6 files, so there's 1 run.
  - **Elapsed time:** 8min 9.8sec

- Final files:

  - `doctable.bin`: 238MB;	average length:1170.18
  - `lexicon.bin`: 755MB;	Total number of terms: 25,620,724
  - `index.bin`: 3.31GB

**Conclusion**

No matter what's the size of buffer, total bytes are the same, so the total time for disk reading or writing with fixed disk speed should be the same. The major difference is disk access delay, but it's not significant especially when using fast SSD.



### Query Processing Result

- Data: output files from index building. `doctable.bin`, `lexicon.bin` and `index.bin`

Here're some query strings aimed for testing different scenarios.

- Disjunctive query with the most frequent terms

  - `[Enter] to || the`
  - `[Formatted query] "(to) || (the)"`
  - `[Query time] 0.584 seconds` (before loaded into LRU Cache)
  - `[Query time] 0.556 seconds` (after loaded into LRU Cache)
- Conjunctive query with the most frequent terms

  - `[Enter] to the`
  - `[Formatted query] "to && the"`
  - `[Query time] 0.403 seconds`
- An regular query

  - `[Enter] male and female`
  - `[Formatted query] "male && and && female"`
  - `[Query time] 0.243 seconds` (before loaded into LRU Cache)
  - `[Query time] 0.090 seconds` (after loaded into LRU Cache)
- A complex query with both disjunction and conjunction
  - `[Enter] winter || dfjheuiry38478hjk || CDC mask`
  - `[Formatted query] "(winter) || (CDC && mask)"` the other term is invalid
  - `[Query time] 0.040 seconds` (before loaded into LRU Cache)
  - `[Query time] 0.028 seconds` (after loaded into LRU Cache)

**Conclusion**

LRU Cache can greatly improve query performance for average terms. But for disjunctive terms with very long index list, because of the blocks limit mentioned above, speed is the same. Even if the blocks limit is turned off, the bottleneck isn't file I/O.

