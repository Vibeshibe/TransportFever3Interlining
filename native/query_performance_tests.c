/* Synthetic native layouts, using actual copied-memory reads through Wine.
 * Measures query work only; it is not an in-game frame-time benchmark. */
#define main query_regression_main
#include "query_tests.c"
#undef main
#define RUNS 2000
static int compare_samples(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}
static void benchmark(unsigned scan) {
    static unsigned char sparse_rows[1100][24];
    static uint64_t sparse_masks[1100][2];
    double samples[RUNS], sum = 0;
    unsigned peak_queries = 0, peak_reads = 0, peak_bytes = 0;
    LARGE_INTEGER frequency; CHECK(QueryPerformanceFrequency(&frequency));
    fixture();
    if (scan) {
        vector_at(world + 0x90, sparse_rows, sizeof(sparse_rows));
        vector_at(world + 0xc0, sparse_masks, sizeof(sparse_masks));
    }
    for (unsigned n = 0; n < RUNS; ++n) {
        LARGE_INTEGER begin, end; QueryPerformanceCounter(&begin);
        tfx_reader r = reader();
        if (scan) {
            tfx_line_page page = {.struct_size = sizeof(page)};
            CHECK(tfx_query_scan_lines(&r, 0, &page) == TFX_OK);
            CHECK(!page.count && page.next_entity == 1024);
        } else {
            for (unsigned q = 0; q < 64; ++q) {
                tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
                CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK);
                CHECK(leg.person == 1 && leg.line == 2);
            }
        }
        QueryPerformanceCounter(&end);
        samples[n] = (double)(end.QuadPart - begin.QuadPart) * 1e6 / (double)frequency.QuadPart;
        sum += samples[n];
        CHECK(r.queries_left <= 64 && r.reads_left <= 8192 && r.bytes_left <= 1024*1024);
        unsigned q = 64-r.queries_left, reads = 8192-r.reads_left, bytes = 1024*1024-r.bytes_left;
        if (q > peak_queries) peak_queries = q;
        if (reads > peak_reads) peak_reads = reads;
        if (bytes > peak_bytes) peak_bytes = bytes;
    }
    qsort(samples, RUNS, sizeof(*samples), compare_samples);
    printf("PERF scenario=%s samples=%u mean_us=%.3f p50_us=%.3f p95_us=%.3f p99_us=%.3f max_us=%.3f peak_queries=%u peak_reads=%u peak_bytes=%u\n",
        scan ? "sparse_1024_entity_page" : "64_waiting_queries", RUNS, sum/RUNS,
        samples[RUNS/2-1], samples[RUNS*95/100-1], samples[RUNS*99/100-1], samples[RUNS-1],
        peak_queries, peak_reads, peak_bytes);
}
int main(void) {
    benchmark(1); benchmark(0);
    puts("PASS: native query costs measured with ReadProcessMemory; synthetic layouts, not in-game FPS.");
    return 0;
}
