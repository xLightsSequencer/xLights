#ifndef RENDERUTILS_H
#define RENDERUTILS_H

class NodeRange 
{
public:

    unsigned int start;
    unsigned int end;

    NodeRange(unsigned int s, unsigned int e) : start(s), end(e) {}
    
    bool operator<(const NodeRange &r) const {
        return start < r.start;
    }

    bool Overlaps(const NodeRange &r) const {
        
        if (end < r.start) {
            return false;
        }
        if (r.end < start) {
            return false;
        }
        if (start > r.end) {
            return false;
        }
        if (r.start > end) {
            return false;
        }
        
        if (start >= r.start && start <= r.end) {
            //start is in the range
            return true;
        }
        if (end >= r.start && start <= r.end) {
            //end is in the range
            return true;
        }
        if (r.start >= start && r.start <= end) {
            //start is in the range
            return true;
        }
        if (r.end >= start && r.start <= end) {
            //end is in the range
            return true;
        }
        return false;
    }
};

#endif