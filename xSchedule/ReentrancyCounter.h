#ifndef REENTRANCYCOUNTER_H
#define REENTRANCYCOUNTER_H

class ReentrancyCounter
{
	int& _ref;
    std::recursive_mutex _lock;

	public:
		ReentrancyCounter(int& ref) : _ref(ref)
		{
            std::unique_lock<std::recursive_mutex> locker(_lock);
		    ++_ref;
		}
		virtual ~ReentrancyCounter()
		{
            std::unique_lock<std::recursive_mutex> locker(_lock);
            --_ref;
			wxASSERT(_ref >= 0);
		}
		bool SoleReference()
		{
            std::unique_lock<std::recursive_mutex> locker(_lock);
            return _ref == 1;
		}
};

#endif
