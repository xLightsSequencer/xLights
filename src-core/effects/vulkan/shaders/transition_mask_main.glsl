// Common main() for the mask ("type 1") transition kernels.  The includer
// defines void maskValue(uint ix, uint iy)
// which computes the value and calls maskWrite itself (some transitions
// write to a permuted position).  The mask is a byte array in
// column-major order (x * height + y — "masks are different order than
// pixels"); bytes are updated through 32-bit words with an And/Or atomic
// pair — each byte has exactly one writer, so the pair cannot corrupt
// neighbors.
void maskWrite(uint sidx, uint value) {
    uint word = sidx >> 2u;
    uint shift = (sidx & 3u) * 8u;
    atomicAnd(maskWords[word], ~(0xFFu << shift));
    atomicOr(maskWords[word], (value & 0xFFu) << shift);
}

void main() {
    uvec2 index = gl_GlobalInvocationID.xy;
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    maskValue(index.x, index.y);
}
