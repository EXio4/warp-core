
int rgba(int r, int g, int b, int a = 255) {
    return
        (a << 24) |  // A
        (b << 16) |  // B
        (g <<  8) |  // G
         r;          // R
}