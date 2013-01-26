#ifndef QUEUE_HPP
#define QUEUE_HPP

template <class T>
class Queue {
  public:
    Queue();
    Queue(const size_t);
    void initialize(const size_t);
    int lock();
    int unlock();
    void push(const T&);
    const T &front() const;
    void pop();
    const size_t &size() const;
    const size_t &capacity() const;
    bool empty() const;
  private:
    T *beginning;
    T *end;
    T *_front;
    T *free;
    size_t _size;
    size_t _capacity;
    pthread_mutex_t _lock;
    void _initialize(const size_t&);
};

template <class T>
Queue <T>::Queue() {}

template <class T>
Queue <T>::Queue(const size_t size) {
  _initialize(size);
}

template <class T>
void Queue <T>::initialize(const size_t size) {
  _initialize(size);
}

template <class T>
int Queue <T>::lock() {
  return pthread_mutex_lock(&_lock);
}

template <class T>
int Queue <T>::unlock() {
  return pthread_mutex_unlock(&_lock);
}

template <class T>
void Queue <T>::push(const T &object) {
  if (_size < _capacity) {
    *free = object;
    ++free;
    ++_size;
    if (free == end) {
      free = beginning;
    }
  }
}

template <class T>
const T &Queue <T>::front() const {
  return *_front;
}

template <class T>
void Queue <T>::pop() {
  if (_size) {
    ++_front;
    if (_front == end) {
      _front = beginning;
    }
    --_size;
  }
}

template <class T>
const size_t &Queue <T>::size() const {
  return _size;
}

template <class T>
const size_t &Queue <T>::capacity() const {
  return _capacity; 
}

template <class T>
bool Queue <T>::empty() const {
  return (_size == 0);
}

template <class T>
void Queue <T>::_initialize(const size_t &size) {
  pthread_mutex_init(&_lock, NULL);
  beginning = new T[size];
  end = beginning + size;
  _front = beginning;
  free = beginning;
  _size = 0;
  _capacity = size;
}

#endif
