#ifndef ISTRREADER_FILE_READER_HPP
#define ISTRREADER_FILE_READER_HPP

#include <fstream>
#include <arpa/inet.h>

#include "../FileReader.hpp"
#include "ISTRRecord.h"

namespace vn {
    namespace arl {
        namespace shared {

            template <typename FileEnumeratorType>
            class ISTRReader : public FileReader <FileEnumeratorType, ISTRRecord> {
            private:
                std::ifstream curFile;
                std::string* nextHeader;

            public:
                explicit ISTRReader(const FileEnumeratorType& fEnum);

                ~ISTRReader();

                /// \brief if this method returns a NULL pointer but contains no errors
                ///        then there are no more available records.
                virtual boost::shared_ptr <Serializable <ISTRRecord> > read();
            };

            template <typename FileEnumeratorType>
            ISTRReader<FileEnumeratorType>::ISTRReader(const FileEnumeratorType& fEnum) :
            FileReader<FileEnumeratorType, ISTRRecord>(fEnum), curFile(NULL), nextHeader(NULL) {
            }

            template <typename FileEnumeratorType>
            ISTRReader<FileEnumeratorType>::~ISTRReader() {
                if(nextHeader != NULL) {
                    delete nextHeader;
                    nextHeader = NULL;
                }

                if (curFile.is_open()) {
                    curFile.close();
                }
            }

            template <typename FileEnumeratorType>
            boost::shared_ptr<Serializable <ISTRRecord> > ISTRReader<FileEnumeratorType>::read() {
                using namespace std;
                using namespace boost;

                Serializable <ISTRRecord>* newRecord;

                // open the next file to read.
                if (!curFile.is_open()) {
                    if (this->fIt == this->fEnum.end()) {
                        // Don't bother, no more files.
                        return shared_ptr<Serializable <ISTRRecord> >();
                    }

                    const char* filename = (this->fIt)->file_string().c_str();
                    curFile.open(filename, ios::in);

                    string header;
                    getline(curFile, header);

                    cout << header << endl;

                    if (curFile.fail()) {
                        this->_error = true;
                        this->_errorMsg = string("Failed to open file ") + filename;

                        return shared_ptr<Serializable <ISTRRecord> >();
                    }

                    this->fIt++;
                }

                {
                    bool done = false;
                    newRecord = new ISTRRecord();

                    string tmp;

                    // read the header.
                    if(nextHeader == NULL) {
                        getline(curFile, tmp);
                    } else {
                        tmp = *nextHeader;
                        delete nextHeader;
                        nextHeader = NULL;
                    }
                    
                    newRecord->unserialize(tmp);

                    // the rest of the crap.
                    while(!done && curFile) {
                        getline(curFile, tmp);

                        if(tmp.find(string("---- ")) == 0) {
                            done = true;
                            nextHeader = new string(tmp);
                        } else {
                            newRecord->unserialize(tmp);
                        }
                    }
                }

                if (curFile.eof()) {
                    curFile.close();
                }

                return shared_ptr<Serializable <ISTRRecord> >(newRecord);
            }

        } // namespace shared
    } // namespace arl
} // namespace vn

#endif