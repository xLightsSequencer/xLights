// tex2D over the packed-uint pixel buffers (bindings declared by includer).
uvec4 tex2Dsrc(float s, float t) {
    s = clamp(s, 0.0, 1.0);
    t = clamp(t, 0.0, 1.0);
    int x = int(xl_round(s * float(data.width - 1u)));
    int y = int(xl_round(t * float(data.height - 1u)));
    return unpackPx(src[y * int(data.width) + x]);
}
uvec4 tex2Dprev(float s, float t) {
    s = clamp(s, 0.0, 1.0);
    t = clamp(t, 0.0, 1.0);
    int x = int(xl_round(s * float(data.pWidth - 1u)));
    int y = int(xl_round(t * float(data.pHeight - 1u)));
    return unpackPx(prev[y * int(data.pWidth) + x]);
}
