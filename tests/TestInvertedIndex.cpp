#include "TestHelper.h"
#include "InvertedIndex.h"

std::string Sample1 = "The Hobbit was first published in September 1937. Its 1951 second edition (fifth impression) contains a significantly revised portion of Chapter V, “Riddles in the Dark,” which brings the story of The Hobbit more in line with its sequel, The Lord of the Rings, then in progress. Tolkien made some further revisions to the American edition published by Ballantine Books in February 1966, and to the British third edition (sixteenth impression) published by George Allen & Unwin later that same year.";
std::string Sample2 = "For the 1995 British hardcover edition, published by HarperCollins, the text of The Hobbit was entered into word-processing files, and a number of further corrections of misprints and errors were made. Since then, various editions of The Hobbit have been generated from that computerized text file. For the present text, that file has been compared again, line by line, with the earlier editions, and a number of further corrections have been made to present a text that, as closely as possible, represents Tolkien’s final intended form.";
std::string Sample3 = "Readers interested in details of the changes made at various times to the text of The Hobbit are referred to Appendix A, “Textual and Revisional Notes,” of The Annotated(1988), and J. R. R. Tolkien: A Descriptive Bibliography by Wayne G. Hammond, with the assistance of Douglas A. Anderson (1993).";

void display_entry_vector(const std::vector<Entry>& entries) {
    for (auto& entry : entries) {
        std::cout << "doc_id: " << entry.doc_id << ", count: " << entry.count << std::endl;
    }
}

bool have_match(const Entry& value, std::vector<Entry>& entries) {
    for (auto& entry : entries) {
        if (entry == value) return true;
    }
    return false;
}

void TestInvertedIndexFunctionality(
        const std::vector<std::string>& docs,
        const std::vector<std::string>& requests,
        const std::vector<std::vector<Entry>>& expected) {

    std::vector<std::string> test_files;
    make_test_files(docs, test_files);

    for (int i = 0; i < 100; ++i) {
        std::vector<std::vector<Entry>> result;
        InvertedIndex idx;
        idx.update_document_base(test_files, true);

        for (auto &request: requests) {
            std::vector<Entry> word_count = idx.get_word_count(request);
            result.push_back(word_count);
        }

        REQUIRE(result.size() == expected.size());

        for (int j = 0; j < expected.size(); ++j) {
            auto &exp = expected[j];
            auto &res = result[j];
            REQUIRE(exp.size() == res.size()); // Amount of answers is equal for each requests

            for (auto &exp_entry: exp) {
                bool success = have_match(exp_entry, res);
                if (!success) {
                    std::cout << "Expected:" << std::endl;
                    display_entry_vector(exp);
                    std::cout << "Got: " << std::endl;
                    display_entry_vector(res);
                }
                REQUIRE(success);
            }
        }
    }

    remove_test_files(test_files);
}

TEST_CASE("InvertedIndex_TestBasic") {
    const std::vector<std::string> docs = {
            "london is the capital of great britain",
            "big ben is the nickname for the Great bell of the striking clock"
    };
    const std::vector<std::string> requests = {"london", "the"};
    const std::vector<std::vector<Entry>> expected = {
            {
                    {0, 1}
            },
            {
                    {1, 3}, {0, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST_CASE("InvertedIndex_TestBasic2") {
    const std::vector<std::string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const std::vector<std::string> requests = {"milk", "water", "cappuccino"};
    const std::vector<std::vector<Entry>> expected = {
            {
                    {0, 4}, {1, 1}, {2, 5}
            },
            {
                    {0, 3}, {1, 2}, {2, 5}
            },
            {
                    {3, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST_CASE("InvertedIndex_IndexExtension") {
    std::vector<std::string> filenames;
    make_test_files({Sample1, Sample2, Sample3}, filenames);

    std::map<size_t, std::string> docs1 = {
            {0, filenames[0]},
            {1, filenames[1]},
    };

    std::map<size_t, std::string> docs2 = {
            {0, filenames[1]},
            {1, filenames[2]},
    };

    for (int i = 0; i < 100; ++i) {
        InvertedIndex idx;
        idx.extend_document_base(docs1, true);
        idx.extend_document_base(docs2, true);
    }

    remove_test_files(filenames);
}

TEST_CASE("InvertedIndex_IndexationPerformance") {
    std::vector<std::string> filenames;
    make_test_files({Sample1, Sample2, Sample3}, filenames);

    std::vector<std::string> docs = {
            filenames[0],
            filenames[1],
            filenames[2],
            filenames[0],
            filenames[1],
            filenames[2],
            filenames[0],
            filenames[1],
            filenames[2],
            filenames[0],
            filenames[1],
            filenames[2],
            filenames[0],
            filenames[1],
            filenames[2],
            filenames[0],
    };

    BENCHMARK("Indexation: separated access, 1 thread") {
        InvertedIndex idx;
        idx.max_indexation_threads = 1;
        idx.indexation_method = InvertedIndex::SeparatedAccess;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: independent dicts, 1 thread") {
        InvertedIndex idx;
        idx.max_indexation_threads = 1;
        idx.indexation_method = InvertedIndex::IndependentDicts;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: separated access, 4 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 4;
        idx.indexation_method = InvertedIndex::SeparatedAccess;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: independent dicts, 4 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 4;
        idx.indexation_method = InvertedIndex::IndependentDicts;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: separated access, 8 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 8;
        idx.indexation_method = InvertedIndex::SeparatedAccess;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: independent dicts, 8 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 8;
        idx.indexation_method = InvertedIndex::IndependentDicts;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: separated access, 16 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 16;
        idx.indexation_method = InvertedIndex::SeparatedAccess;
        idx.update_document_base(docs, true);
    };

    BENCHMARK("Indexation: independent dicts, 16 threads") {
        InvertedIndex idx;
        idx.max_indexation_threads = 16;
        idx.indexation_method = InvertedIndex::IndependentDicts;
        idx.update_document_base(docs, true);
    };

    remove_test_files(filenames);
}

TEST_CASE("InvertedIndex_MissingWord") {
    const std::vector<std::string> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const std::vector<std::string> requests = {"m", "statement"};
    const std::vector<std::vector<Entry>> expected = {
            {
            },
            {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST_CASE("InvertedIndex_URLContent") {
    const std::vector<std::string> docs = {"https://example.com/"};
    const std::string request = "Domain";

    InvertedIndex idx;
    idx.update_document_base(docs, true);

    std::vector<Entry> word_count = idx.get_word_count(request);
    REQUIRE((word_count.empty() || word_count.size() == 1));

    if (!word_count.empty()) {
        REQUIRE(word_count[0].count == 2);
    }
}