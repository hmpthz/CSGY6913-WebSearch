## Web Search Engine Assignment2 - Inverted Index

**Haozhong Zheng, hz2675@nyu.edu**



### Summary

**Programming Language**

My program consists of both C++ and python code.

- The C++ part has all data structures needed. These structures have major functions used for merging, file reading and writing. C++ part provides python interface, it should be compiled to dynamic link library for python to call.

- The python part only has one file `IndexBuilder.py`, which is where the program executes. It basically parse the command line arguments, load C++ DLL, then run certain procedures to build inverted index.

  Using python is because it's easier to implement trivial things that aren't strongly related to inverted index.

**Algorithm and Features**

The program uses merging-subindexes algorithm instead of merging intermediate postings. Also, the inverted indexes and the corresponding lexicon terms are arranged in alphabetical orders (more precisely, byte order, because there're utf-8 characters).

Therefore, I divide the whole building process into two steps:

- The first step would parse the giving `fulldocs-new.trec` file, tokenize documents. Then C++ part would build doc table, convert tokens to postings, then to inverted index, build lexicon and write file to disk when index buffer reaches memory limit.

- The second step would perform n-way merge to build the final index which should be I/O efficient if the buffer size and N is properly set.

The program can conveniently set the number of n-way merge, size of input and output buffer by command line. It supports utf-8 text file format or compressed binary format by compiler flag.

**Environment and Compile**

- `python`: python3, with module `nltk`

- `C++`: I use msvc on windows, but other compilers on other platforms should work well too. The compiler should build dynamic link library.

  If preprocessor defines `TEXT_FILE`, then the file is utf-8 text format, otherwise it's compressed binary format.



### Command Line Argument

Execute the IndexBuilder by command `python IndexBuilder.py` with arguments. As I mentioned above, the program is divided into two steps, so it should execute two times with different arguments.

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



### File and Data Structures

**DocTable**

- `DocTable.h`, `DocTable.cpp`

- doc_id is the order the pages are parsed

- An item of DocTable has three elements: (start offset of document in input file, number of terms in this document, url)

  `DocTableItem = std::tuple<uint64_t, uint32_t, std::string>`

- Average number of terms in the table

- `std::vector<DocTableItem>`

- File format:

```
text format:
<avg_len> (\n)
<start offset> (space) <# of terms> (space) <url> (\n)
binary format:
<4-avg_len>
<8-start offset> <4-# of terms> <str-url> <1-\0>
```

**Lexicon**

- `Lexicon.h`, `Lexicon.cpp`

- An item of Lexicon has three elements: (start offset in index file of this term, end offset, number of documents containing this term)

  `LexiconValue = std::tuple<uint64_t, uint64_t, uint32_t>`

- Use ordered map to maintain alphabetical order of terms,

  `std::map<std::string, LexiconValue>`

- File format:

```
text format:
<term> (space) <start offset> (space) <end offset> (space) <n_docs> (\n)
binary format:
<str-term> <1-\0> <8-start offset> <8-end offset> <4-n_docs>
```

**PostingsBuffer**

- `Lexicon.h`, `Lexicon.cpp`

- A single posting has doc_id and frequency.

  `std::tuple<uint32_t, uint32_t>`

- PostingsBuffer is an intermediate data structure kept in memory that used in first step. It estimates the memory cost of its data, when buffer is full, postings should be convert to subindexes.

- Use ordered map to maintain alphabetical order of terms,

  `std::map<std::string, std::vector<Posting>>`

**IndexTerm**

- `IndexTerm.h`, `IndexTerm.cpp`

- It's the data structure for a single inverted index list. Postings are kept in **var-byte compression form and doc_ids are taken differences**. `std::vector<uint8_t>`

- Meta data of a block has three elements: (last doc_id of the block, byte size of compressed doc_id, byte size of frequency)

  `BlockMetaData = std::tuple<uint32_t, uint16_t, uint16_t>`

  `std::vector<BlockMetaData>`

- An iterator points to the corresponding Lexicon item, so it can get the needed info.

- `get_next` method and `append` method. Used for transferring (merging) postings of IndexTerm from input buffer to IndexTerm from output buffer.

- This data structure includes a block of uncompressed postings cache used for `get_next` and `append` method. During the transfer, it maintains the cache by load or unload from compressed bytes.

- File format:

```
text format: uncompressed ascii numbers
a block: <last_did> (space) <# of postings in this block> (space) [<doc_id> (space) ....] [<frequency> (space) ....] (\n)
terminator: <'#'> (\n)
binary format:
a block: <4-last_did> <2-did_size> <2-freq_size> <did_size-CompressedBytes> <freq_size-CompressedBytes>
terminator: <1-0>
```

**IndexBuffer**

- `IndexBuffer.h`, `IndexBuffer.cpp`

- A linked list of IndexTerm which is easier to erase and insert than array.

  `std::list<IndexTerm>`

- Each IndexBuffer is associated to a Lexicon object. For simplicity, memory cost of Lexicon is not counted. To save memory, Lexicon item will be removed or added along with IndexTerm list item.

- While inserting postings, IndexTerm object would compute the estimated memory size, then add into IndexBuffer.

**Others**

- `helper.h`, `helper.cpp`: CompressedBytes class with `compress`, `decompress`, `difference` and `undifference` methods. Also there're some global parameters.
- `PyInterface.h`, `PyInterface.cpp`: Major functions exposed to python.
- `IndexBuilder.py`: Run the program.



### Procedures Description

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

<div style="page-break-after: always;"></div>

### Limitations

1. Estimate the memory cost of an object is difficult, I count the size in primitive way so it's inaccurate.
2. `nltk.tokenize` doesn't support other languages, and numbers are hard to decide how to tokenize. Since python has many great language processing modules, this problem can be improved further.
3. No multiprocessing during merging to improve the performance. This is also easy to solve because multiprocessing in python is easy to implement but I don't have enough time.



### Result

- Dataset: https://microsoft.github.io/msmarco/TREC-Deep-Learning-2020

  [fulldocs-new.trec](https://msmarco.blob.core.windows.net/msmarcoranking/msmarco-docs.trec.gz);	size: 21.4GB;	number of documents: 3,213,835
  
- Step1:

  - input buffer: 1000MB;	output buffer: 500MB
  - Total number of output files: 16
  - **Elapsed time:** 71min 31.4sec

- Step2:

  - input buffer: 2000MB;	output buffer: 500MB
  - **4-way merge:** 16 files, so there're 2 runs.
  - **Elapsed time:** 12min 23.3sec

- Final files:

  - `doctable.bin`: 238MB
  - `lexicon.bin`: 755MB;	Total number of terms: 25,620,724
  - `index.bin`: 3.31GB

