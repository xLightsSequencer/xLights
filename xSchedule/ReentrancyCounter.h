#ifndef REENTRANCYCOUNTER_H
#define REENTRANCYCOUNTER_H

class ReentrancyCounter
{
	int& _ref;
    std::recursive_mutex _lock;

	public:
		ReentrancyCounter(int& ref) : _ref(ref)
		{
            std::lock_guard<std::recursive_mutex> locker(_lock);
		    ++_ref;
		}
		virtual ~ReentrancyCounter()
		{
            std::lock_guard<std::recursive_mutex> locker(_lock);
            --_ref;
			wxASSERT(_ref >= 0);
            if (_ref < 0) _ref = 0;
		}
		bool SoleReference()
		{
            std::lock_guard<std::recursive_mutex> locker(_lock);
            return _ref == 1;
		}
};

#endif
