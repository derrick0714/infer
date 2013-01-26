#ifndef INFER_INCLUDE_INFERFILEWRITER_HPP_
#define INFER_INCLUDE_INFERFILEWRITER_HPP_

#include <map>

#include "ErrorStatus.hpp"
#include "DataTypeTraits.hpp"
#include "StrftimeWriteEnumerator.hpp"

/// \todo efficiently keep multiple files open, and figure out a clean way to
/// close old ones. Ie. have a maxOpenedFiles parameter, or maybe a timeout of
/// some sort. Maybe even have a different class for each of those. -Justin
template <typename WriterType,
		  typename EnumeratorType = StrftimeWriteEnumerator<typename WriterType::value_type> >
class InferFileWriter {
  public:
	typedef writer_type_tag category;
	typedef typename WriterType::value_type value_type;

	/// \brief Constructor
	explicit InferFileWriter(
				boost::shared_ptr<EnumeratorType> enumerator,
				uint32_t maxOpenWriters = 2);
	
	/// \brief Destructor
	~InferFileWriter() {
		typename std::map<uint32_t, WriterType *>::iterator i(_writers.begin());
		
		while (i != _writers.end()) {
			i->second->close();
			delete i->second;
			_writers.erase(i++);
		}
	}

	/// \brief Write an object
	/// \param obj the object to write
	template <typename TPointer>
	ErrorStatus write(const TPointer obj) {
		return _write(obj, typename WriterType::category());
	}

	/// \brief Close the encapsulated file writer
	/// \returns true if the file writer closed successfully
	ErrorStatus close() {
		ErrorStatus errorStatus;
		typename std::map<uint32_t, WriterType *>::iterator i(_writers.begin());
		
		while (i != _writers.end()) {
			errorStatus = i->second->close();
			if (errorStatus != E_SUCCESS) {
				return errorStatus;
			}
			delete i->second;
			_writers.erase(i++);
		}

		return E_SUCCESS;
	}

  private:
	template <typename TPointer>
	ErrorStatus _write(const TPointer obj, file_writer_type_tag);

	/// \brief The currently opened file writer
	std::map <uint32_t, WriterType *> _writers;

	/// \brief The write enumerator to use for determining file names
	boost::shared_ptr <EnumeratorType> _enumerator;

	const uint32_t _maxOpenWriters;
};

template <typename WriterType,
		  typename EnumeratorType>
InferFileWriter<WriterType, EnumeratorType>::
InferFileWriter(boost::shared_ptr<EnumeratorType> enumerator,
				uint32_t maxOpenWriters)
	:_writers(),
	 _enumerator(enumerator),
	 _maxOpenWriters(maxOpenWriters)
{
}

template <typename WriterType,
		  typename EnumeratorType>
template <typename TPointer>
ErrorStatus InferFileWriter<WriterType, EnumeratorType>::
_write(const TPointer obj, file_writer_type_tag) {
	ErrorStatus errorStatus;
	typename std::map<uint32_t, WriterType *>::iterator it(_writers.find(_enumerator->getIndex(*obj)));
	if (it == _writers.end()) {
		// the correct file is not open.
		// close the oldest file if the max number are already open
		if (_writers.size() == _maxOpenWriters) {
			errorStatus = _writers.begin()->second->close();
			if (errorStatus != E_SUCCESS) {
				return errorStatus;
			}
			delete _writers.begin()->second;
			_writers.erase(_writers.begin());
		}

		// open the file to be written to
		it = _writers.insert(std::make_pair(_enumerator->getIndex(*obj), new WriterType)).first;
		errorStatus = it->second->open(_enumerator->getFileName(*obj));
		if (errorStatus == E_NOPARENT) {
			boost::filesystem::create_directories(_enumerator->getFileName(*obj).parent_path());
			errorStatus = it->second->open(_enumerator->getFileName(*obj));
		}
		if (errorStatus != E_SUCCESS) {
			return errorStatus;
		}
	}

	// it now points to the correct, open file.
	return it->second->write(obj);
}

#endif
