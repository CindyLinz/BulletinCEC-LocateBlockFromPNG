#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

//#define DEBUG

#define BLOCK_INC (64)
#define NOISE_THRESHOLD (20)
#define SMALL_TABLE_THRESHOLD (8)

// board pixel bits usage:
const unsigned char TRY_DIR  =   1; // current trying h or v direction: 0.h, 1.v
const unsigned char TRY_STEP =   2; // current trying step: 0.dec, 1.inc
const unsigned char PAR_DIR  =   4; // parent h or v direction: 0.h, 1.v
const unsigned char PAR_STEP =   8; // parent step: 0.dec, 1.inc
const unsigned char VISITED  =  16; // is visited pixel: 0.no, 1.yes
const unsigned char IMG      =  32; // 0.non-white, 1.white
const unsigned char IS_FRAME = 128; // is frame: 0.no, 1.yes

inline int min(int a, int b){ return a<b ? a : b; }
inline int max(int a, int b){ return a>b ? a : b; }

inline int move_cursor(int width, int height, unsigned char pos_dir, unsigned char pos_step, int cursor){
    if( pos_dir )
        if( pos_step )
            if( cursor / width < height - 1 )
                return cursor + width;
            else
                return -1;
        else
            if( cursor >= width )
                return cursor - width;
            else
                return -1;
    else
        if( pos_step )
            if( cursor % width < width - 1 )
                return cursor + 1;
            else
                return -1;
        else
            if( cursor % width > 0 )
                return cursor - 1;
            else
                return -1;
}

inline void flood_paint(unsigned char * board, int width, int height, int bound_x, int bound_y, int bound_width, int bound_height, char white_frame, int x0, int y0){
    int cursor0 = y0 * width + x0;
    int cursor = cursor0;
    if( (white_frame && !(board[cursor0] & IMG)) || (!white_frame && (board[cursor0] & IMG)) )
        return;

#ifdef DEBUG
    fprintf(stderr, "flood_paint(%d) %dx%d+%d+%d at (%d,%d)\n", (int) white_frame, bound_width, bound_height, bound_x, bound_y, x0, y0);
#endif

    while(1){
        unsigned char pixel = board[cursor];
#ifdef DEBUG2
        {
            int x = cursor % width;
            int y = cursor / width;
            char par, try;
            if( pixel & TRY_DIR )
                if( pixel & TRY_STEP )
                    try = 'v';
                else
                    try = '^';
            else
                if( pixel & TRY_STEP )
                    try = '>';
                else
                    try = '<';
            if( pixel & PAR_DIR )
                if( pixel & PAR_STEP )
                    par = 'v';
                else
                    par = '^';
            else
                if( pixel & PAR_STEP )
                    par = '>';
                else
                    par = '<';
            fprintf(stderr, "cursor=%d (%d, %d) par=%c try=%c\n", cursor, x, y, par, try);
        }
#endif
        if( pixel & VISITED ){
            if( pixel & TRY_DIR )
                if( pixel & TRY_STEP ){
                    if( cursor==cursor0 )
                        break;
                    else
                        cursor = move_cursor(width, height, pixel&PAR_DIR, pixel&PAR_STEP, cursor); // without check
                }
                else{
                    board[cursor] |= TRY_STEP;
                    int cursor2 = move_cursor(width, height, 1, 1, cursor);
                    if( cursor2>=0 && !(board[cursor2] & VISITED) ){
                        cursor = cursor2;
                        board[cursor2] |= PAR_DIR;
                    }
                }
            else
                if( pixel & TRY_STEP ){
                    board[cursor] |= TRY_DIR;
                    board[cursor] &= ~TRY_STEP;
                    int cursor2 = move_cursor(width, height, 1, 0, cursor);
                    if( cursor2>=0 && !(board[cursor2] & VISITED) ){
                        cursor = cursor2;
                        board[cursor2] |= PAR_DIR | PAR_STEP;
                    }
                }
                else{
                    board[cursor] |= TRY_STEP;
                    int cursor2 = move_cursor(width, height, 0, 1, cursor);
                    if( cursor2>=0 && !(board[cursor2] & VISITED) ){
                        cursor = cursor2;
                    }
                }
        }
        else{
            if( (white_frame && (board[cursor] & IMG)) || (!white_frame && !(board[cursor] & IMG)) ){
                board[cursor] |= VISITED | IS_FRAME;
                int cursor2 = move_cursor(width, height, 0, 0, cursor);
                if( cursor2>=0 && !(board[cursor2] & VISITED) ){
                    cursor = cursor2;
                    board[cursor2] |= PAR_STEP;
                }
            }
            else{
                board[cursor] |= VISITED;
                cursor = move_cursor(width, height, pixel&PAR_DIR, pixel&PAR_STEP, cursor); // without check
            }
        }
    }
}

inline void paint_frame(unsigned char * board, int width, int height, int bound_x, int bound_y, int bound_width, int bound_height, char white_frame){
    {
        unsigned char * p = board + bound_y * width + bound_x;
        for(int y=bound_y; y<bound_y+bound_height; ++y){
            for(int x=bound_x; x<bound_x+bound_width; ++x, ++p)
                *p &= IMG;
            p = p - bound_width + width;
        }
    }

    {
        unsigned char * p1 = board + bound_y * width + bound_x;
        unsigned char * p2 = board + (bound_y+bound_height-1) * width + bound_x;
        for(int x=bound_x; x<bound_x+bound_width; ++x, ++p1, ++p2){
            if( !(*p1 & VISITED) )
                flood_paint(board, width, height, bound_x, bound_y, bound_width, bound_height, white_frame, x, bound_y);
            if( !(*p2 & VISITED) )
                flood_paint(board, width, height, bound_x, bound_y, bound_width, bound_height, white_frame, x, bound_y+bound_height-1);
        }
    }

    {
        unsigned char * p1 = board + bound_y * width + bound_x;
        unsigned char * p2 = board + bound_y * width + bound_x+bound_width-1;
        for(int y=bound_y; y<bound_y+bound_height; ++y, p1+=width, p2+=width){
            if( !(*p1 & VISITED) )
                flood_paint(board, width, height, bound_x, bound_y, bound_width, bound_height, white_frame, bound_x, y);
            if( !(*p2 & VISITED) )
                flood_paint(board, width, height, bound_x, bound_y, bound_width, bound_height, white_frame, bound_x+bound_width-1, y);
        }
    }

#ifdef DEBUG
    for(int y=bound_y; y<bound_y+bound_height && y<bound_y+40; ++y){
        for(int x=bound_x; x<bound_x+bound_width && x<bound_x+150; ++x)
            fprintf(stderr, "%c", (board[y*width+x] & IS_FRAME) ? '#' : '.');
        fprintf(stderr, "\n");
    }
#endif
}

typedef struct {
    int x, y, width, height;
} block_t;

inline int extract_block(char * board, int width, int height, int bound_x, int bound_y, int bound_width, int bound_height, block_t** found_blocks, int found_offset, int* found_capacity){
    int found_n = 0;

    char * p0 = board + bound_y * width + bound_x;
    for(int y0=bound_y; y0<bound_y+bound_height; ++y0){
        for(int x0=bound_x; x0<bound_x+bound_width; ++x0, ++p0)
            if( *p0 >= 0 ){
                char *p = p0;
                int x1 = x0;
                for(x1=x0; x1<bound_x+bound_width && *p>=0; ++x1, ++p)
                    *(unsigned char*)p |= IS_FRAME;

                int y1;
                for(y1=y0+1, p=p0+width; y1<bound_y+bound_height; ++y1, p+=width){
                    if( (x0>0 && *(p-1)>=0) || (x1<width && *(p+(x1-x0))>=0) )
                        break;

                    int x;
                    for(x=x0; x<x1 && *p>=0; ++x, ++p)
                        *(unsigned char*)p |= IS_FRAME;
                    p -= (x - x0);
                    if( x < x1 ){ // in-complete row, put back
                        while( x>x0 ){
                            *(unsigned char*)p &= ~IS_FRAME;
                            --x;
                            ++p;
                        }
                        break;
                    }
                }

                if( x1-x0 > NOISE_THRESHOLD && y1-y0 > NOISE_THRESHOLD ){
                    if( found_offset + found_n >= *found_capacity ){
                        *found_capacity += BLOCK_INC;
                        *found_blocks = realloc(*found_blocks, sizeof(block_t) * *found_capacity);
                    }
                    block_t * block_slot = *found_blocks + found_offset + found_n;
                    block_slot->x = x0;
                    block_slot->y = y0;
                    block_slot->width = x1-x0;
                    block_slot->height = y1-y0;
                    ++found_n;
#ifdef DEBUG2
                    fprintf(stderr, "%d %d %d %d\n", x0, y0, x1-x0, y1-y0);
#endif
                }
            }
        p0 = p0 - bound_width + width;
    }

    return found_n;
}

void find_block(unsigned char * board, int width, int height, int bound_x, int bound_y, int bound_width, int bound_height, block_t** found_blocks, int found_offset, int* found_capacity){
    paint_frame(board, width, height, bound_x, bound_y, bound_width, bound_height, 1);
    int found_black_n = extract_block((char*)board, width, height, bound_x, bound_y, bound_width, bound_height, found_blocks, found_offset, found_capacity);
#ifdef DEBUG
    fprintf(stderr, "found_black_n=%d\n", found_black_n);
    for(int i=0; i<found_black_n; ++i){
        block_t * curr_black_block = *found_blocks + found_offset + i;
        fprintf(stderr, "  %d %d %d %d\n", curr_black_block->x, curr_black_block->y, curr_black_block->width, curr_black_block->height);
    }
#endif
    for(int i=0; i<found_black_n; ++i){
        block_t * curr_black_block = *found_blocks + found_offset + i;
        paint_frame(board, width, height, curr_black_block->x, curr_black_block->y, curr_black_block->width, curr_black_block->height, 0);
        int found_white_n = extract_block((char*)board, width, height, curr_black_block->x, curr_black_block->y, curr_black_block->width, curr_black_block->height, found_blocks, found_offset+found_black_n, found_capacity);
#ifdef DEBUG
        fprintf(stderr, "found_white_n=%d\n", found_white_n);
        for(int j=0; j<found_white_n; ++j){
            block_t * curr_white_block = *found_blocks + found_offset + found_black_n + j;
            fprintf(stderr, "  %d %d %d %d\n", curr_white_block->x, curr_white_block->y, curr_white_block->width, curr_white_block->height);
        }
#endif

        if( 1 <= found_white_n && found_white_n <= SMALL_TABLE_THRESHOLD ){
            block_t * curr_white_block = *found_blocks + found_offset + found_black_n;
            int bx = curr_white_block->x;
            int by = curr_white_block->y;
            int bw = curr_white_block->width;
            int bh = curr_white_block->height;
            for(int j=1; j<found_white_n; ++j, ++curr_white_block){
                int bx0 = bx;
                int by0 = by;
                bx = min(bx, curr_white_block->x);
                by = min(bx, curr_white_block->y);
                bw = max(bx0+bw, curr_white_block->x+curr_white_block->width) - bx;
                bh = max(by0+bh, curr_white_block->y+curr_white_block->height) - by;
            }
            find_block(board, width, height, curr_white_block->x, curr_white_block->y, curr_white_block->width, curr_white_block->height, found_blocks, found_offset+found_black_n, found_capacity);
        }
        else{
            for(int j=0; j<found_white_n; ++j){
                block_t * curr_white_block = *found_blocks + found_offset + found_black_n + j;
                printf("%d %d %d %d\n", curr_white_block->x, curr_white_block->y, curr_white_block->width, curr_white_block->height);
            }
        }
    }
}

int main(){
    fprintf(stderr, "reading PNG input file from stdin...\n");

    const int header_number = 8;
    {
        png_byte header[header_number];

        if( fread(header, header_number, 1, stdin)!=1 ){
            fprintf(stderr, "read header failed\n");
            return 1;
        }

        if( png_sig_cmp(header, 0, header_number) ){
            fprintf(stderr, "input is not PNG\n");
            return 1;
        }
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if( !png_ptr ){
        fprintf(stderr, "png_create_read_struct fail\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if( !info_ptr ){
        fprintf(stderr, "png_create_info_struct (info_ptr) fail\n");
        return 1;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if( !end_info ){
        fprintf(stderr, "png_create_info_struct (end_info) fail\n");
        return 1;
    }

    if( setjmp(png_jmpbuf(png_ptr)) ){
        fprintf(stderr, "png longjmp..\n");
        return 1;
    }

    png_set_sig_bytes(png_ptr, header_number);
    png_init_io(png_ptr, stdin);

    png_read_png(png_ptr, info_ptr, 0, NULL);
    png_uint_32 width, height;
    int bit_depth, color_type;
    int interlace_type, compression_type, filter_method;
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height,
        &bit_depth, &color_type, &interlace_type,
        &compression_type, &filter_method
    );
    fprintf(stderr, "width=%d, height=%d, bit_depth=%d, color_type=%d\ninterlace_type=%d, compression_type=%d, filter_method=%d\n", (int)width, (int)height, (int)bit_depth, color_type, interlace_type, compression_type, filter_method);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    int size = width * height;
    unsigned char *board = (unsigned char*) malloc(size);

    {
        unsigned char *p = board;
        for(int y=0; y<height; ++y)
            for(int x3=0; x3<width*3; x3+=3, ++p)
                if(
                    row_pointers[y][x3]>200 &&
                    row_pointers[y][x3+1]>200 &&
                    row_pointers[y][x3+2]>200
                )
                    *p = IMG;
                else
                    *p = 0;
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    block_t * found_blocks = NULL;
    int found_capacity = 0;
    find_block(board, width, height, 0, 0, width, height, &found_blocks, 0, &found_capacity);

    return 0;
}
