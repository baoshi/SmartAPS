#ifndef RINGBUF_H
#define RINGBUG_H

template <class T>
class RingBuffer {
public:
	RingBuffer(size_t size) :
		_buf(std::unique_ptr<T[]>(new T[size])),
		_max_size(size)
	{
		_head = 0;
		_tail = 0;
		_full = false;
    }

	void put(T item)
	{
    	_buf[_head] = item;
		if (_full)
		{
			_tail = (_tail + 1) % _max_size;
		}
		_head = (_head + 1) % _max_size;
		_full = _head == _tail;
	}

	T get()
	{
		if (empty())
		{
			return T();
		}

		//Read data and advance the tail (we now have a free space)
		auto val = _buf[_tail];
		_full = false;
		_tail = (_tail + 1) % _max_size;

		return val;
	}

	T operator[](size_t idx) const
	{
		/* DANGEROUS!!! Use when absolute sure i is within range
		if (empty() || i >= size())
		{
			return T();
		}
		*/
		return _buf[(_tail + idx) % _max_size];
	}

	void reset()
	{
		_head = _tail;
		_full = false;
	}


	bool empty() const
	{
		//if head and tail are equal, we are empty
		return (!_full && (_head == _tail));
	}


	bool full() const
	{
		//If tail is ahead the head by 1, we are full
		return _full;
	}


	size_t capacity() const
	{
		return _max_size;
	}


	size_t size() const
	{
		size_t size = _max_size;
		if (!_full)
		{
			if(_head >= _tail)
			{
				size = _head - _tail;
			}
			else
			{
				size = _max_size + _head - _tail;
			}
		}
		return size;
	}

private:
	std::unique_ptr<T[]> _buf;
	size_t _head = 0;
	size_t _tail = 0;
	const size_t _max_size;
	bool _full = false;
};

#endif