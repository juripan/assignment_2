#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_LEN 101
#define SQUARE_SIDE_COUNT 4
#define BASE_TEN 10

#define print_err(msg) fprintf(stderr, msg "\n")
//even though this is defined in <stdlib.h> the compiler decided to complain so I put it here
#define min(a,b) (((a) < (b)) ? (a) : (b))


typedef short int bit;

typedef struct{
    bit* data;
    unsigned int width;
    unsigned int height;
}Image;

typedef struct{
    unsigned row;
    unsigned col;
}Coords;


Image* image_ctor(unsigned height, unsigned width);

Image* load_image_data(char* file_name);

bit* get_bit(Image* img, unsigned row, unsigned col);

void image_dtor(Image** img_ptr);

int arg_parse(int count, char** args);

void print_help();

int test_command(char* file_name);

int test_file_structure(char* file_name);

int get_size(FILE* file, unsigned* rows, unsigned* columns);

int validate_bitmap(FILE* file, unsigned width, unsigned height);

int find_shape(char* file_name, unsigned find_func(Image*, Coords*, Coords*));

unsigned find_vertical_line(Image* img, Coords* start, Coords* end);

unsigned find_horizontal_line(Image* img, Coords* start, Coords* end);

unsigned find_square(Image* img, Coords* start, Coords* end);

bool points_exist(Image* img,  unsigned row, unsigned col, unsigned size);

bool are_lines_valid(Image* img, unsigned row, unsigned col, unsigned size);

void update_max_score(unsigned score, unsigned* max_score, Coords* end, unsigned r, unsigned c);


int main(int argc, char** argv){
    // argc - 1 because we ignore the first argument (the program name)
    if(arg_parse(argc - 1, argv) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}


Image* image_ctor(unsigned height, unsigned width){
    Image* img = malloc(sizeof(Image));
    if(img == NULL) return NULL;

    //one dimensional array used like a 2d array (see get_bit function)
    bit* data = malloc(sizeof(bit) * width * height);
    if(data == NULL) return NULL;

    img->height = height;
    img->width = width;
    img->data = data;

    return img;
}

Image* load_image_data(char* file_name){
    /*
    creates an Image struct on the stack,
    loads file bitmap image into the Image struct from the file,
    returns a pointer to the Image struct
    */

    FILE* file = fopen(file_name, "r");
    if(file == NULL){
        print_err("fopen failed");
        return NULL;
    }
    unsigned col, row;

    //checking the return is redundant since it was checked in test_file_structure
    get_size(file, &col, &row);

    Image* img = image_ctor(col, row);

    if(img == NULL){
        print_err("Image allocation failed");
        fclose(file);
        return NULL;
    }

    char buff;
    unsigned data_index = 0;

    while((buff = fgetc(file)) != EOF){
        // no need to check for whitespaces since it was checked in test_file_structure
        if(buff == '1' || buff == '0'){
            img->data[data_index++] = buff - '0'; // subtracting '0' converts the char into an int
        }
    }
    fclose(file);
    return img;
}

bit* get_bit(Image* img, unsigned row, unsigned col){
    /*
    returns a pointer to a location in the Image->data array
    specified by the row and col parameters,
    returns NULL if the row or col are out of bounds,
    */

    if(row >= img->height || col >= img->width){
        return NULL;
    }
    return &(img->data[(row * img->width) + col]);
}

void image_dtor(Image** img_ptr){
    /*
    frees all memory allocated for Image struct
    and sets the pointer to NULL to prevent accidental use
    */

    free((*img_ptr)->data);
    free(*img_ptr);
    *img_ptr = NULL;
}

int arg_parse(int count, char** args){
    /*
    parses the passed in arguments and triggers the corresponding function,
    returns EXIT_FAILURE if anything goes wrong, otherwise retruns EXIT_SUCCESS 
    */

    if(count > 2){ //note: else ifs aren't needed since there's a return in every if block
        print_err("Too many of arguments");
        return EXIT_FAILURE;
    }
    if(count == 1 && strcmp(args[1], "--help") == 0){
        print_help();
        return EXIT_SUCCESS;
    }
    if(count == 2 && strcmp(args[1], "test") == 0){
        return test_command(args[2]);
    }
    if(count == 2 && strcmp(args[1], "hline") == 0){
        return find_shape(args[2], find_horizontal_line);
    }
    if(count == 2 && strcmp(args[1], "vline") == 0){
        return find_shape(args[2], find_vertical_line);
    }
    if(count == 2 && strcmp(args[1], "square") == 0){
        return find_shape(args[2], find_square);
    }
    print_err("Incorrect arguments");
    return EXIT_FAILURE;
}

void print_help(){
    /*
    prints out all of the commands that this program has
    and their short description to the console,
    */

    const char* HELP_STR = 
    "All commands:\n"
    "   ./figsearch --help\n"
    "       --help: prints out all commands implemented\n"
    "   note: FILENAME has to include the file extention\n"
    "   ./figsearch test FILENAME\n"
    "       test: tests if a given file has the proper formatting\n"
    "       prints out \"Valid\" if the file is the usable, else it throws the error \"Invalid\"\n"
    "   ./figsearch hline FILENAME\n"
    "       hline: finds the first longest horizontal line in the file and prints out its start and end coords\n"
    "   ./figsearch vline FILENAME\n"
    "       vline: finds the first longest vertical line in the file and prints out its start and end coords\n"
    "   ./figsearch square FILENAME\n"
    "       square: finds the first largest square in the file and prints out its start and end coords\n";
    printf("%s", HELP_STR);
}

int test_command(char* file_name){
    /*
    handles the return of the test_file_structure function,
    only needed so the other functions that use test_file_structure
    without printing out "Valid",
    returns the return of test_file_structure
    */

    if(test_file_structure(file_name) == EXIT_FAILURE){
        return EXIT_FAILURE;   
    }
    printf("Valid\n");
    return EXIT_SUCCESS;
}

int test_file_structure(char* file_name){
    /*
    opens a file with the file name that is passed in and checks if it has a valid file structure,
    returns EXIT_FAILURE if the test fails of the file doesn't open, else it returns EXIT_SUCCESS
    */

    FILE* file = fopen(file_name, "r");
    unsigned height, width;
    
    if(file == NULL){
        print_err("Failed to open the file: file probably doesn't exist");
        return EXIT_FAILURE;
    }
    if(get_size(file, &height, &width) == EXIT_FAILURE 
    || validate_bitmap(file, width, height) == EXIT_FAILURE){
        print_err("Invalid");
        fclose(file);
        return EXIT_FAILURE;
    }
    fclose(file);
    return EXIT_SUCCESS;
}

int get_size(FILE* file, unsigned* rows, unsigned* columns){
    /*
    reads and validates the first 2 numbers (size of bitmap),
    writes into the rows and columns addresses the result of parsing the size,
    returns EXIT_FAILURE if it fails,
    */

    char buff[MAX_LINE_LEN]; // the size of the bitmap most likely won't exceed 100 digits
    char* next; // points at the expected next char
    char* end; // points at the end of the string
    int whitespace_count = 0; //there cant be more than one whitespace in between two numbers
    
    unsigned i; //declared here so its available outside of the for loop scope
    for(i = 0; (buff[i] = fgetc(file)) != EOF; i++){
        if(isspace(buff[i])!= 0){
            whitespace_count++;
            if(whitespace_count == 2){ //reads the first two numbers
                break;
            }
        }
    }
    buff[i+1] = '\0';

    *rows = strtol(buff, &next, BASE_TEN);
    
    *columns = strtol(next, &end, BASE_TEN);

    if(*rows == 0 || *columns == 0){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int validate_bitmap(FILE* file, unsigned width, unsigned height){
    /*
    reads the bitmap part of the file and checks if the dimensions fit
    and if the format of the file is correct,
    returns EXIT_FAILURE if the validation failed,
    */

    char buff;
    unsigned bit_count = 0;

    for(unsigned i = 0; (buff = fgetc(file)) != EOF; i++){
        if(i % 2 == 0 && (buff == '1' || buff == '0')){
            bit_count++;
        }
        else if(i % 2 != 0 && isspace(buff)){
            continue;
        }
        else{
            return EXIT_FAILURE;
        }
    }

    if(bit_count != width * height){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int find_shape(char* file_name, unsigned find_func(Image*, Coords*, Coords*)){
    /*
    tests the bitmap validity and finds the largest shape
    (shape specified by the find_func parameter),
    prints the coordinates of the start and the end of the shape found,
    returns EXIT_FAILURE if the test fails
    */

    Coords start = {0};
    Coords end = {0};

    if(test_file_structure(file_name) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    Image* img = load_image_data(file_name);
    if(img == NULL) return EXIT_FAILURE;

    unsigned size = find_func(img, &start, &end);

    if(size == 0){
        printf("Not found\n");
        image_dtor(&img);
        return EXIT_SUCCESS;
    }

    printf("%d %d %d %d\n", start.row, start.col, end.row, end.col);
    image_dtor(&img);
    return EXIT_SUCCESS;
}


unsigned find_vertical_line(Image* img, Coords* start, Coords* end){
    /*
    finds the largest vertical line and writes its 
    coordinates into the start and end parameters,
    returns the length of the line
    */

    unsigned max_score = 0;
    unsigned score = 0;

    for(unsigned c = 0; c < img->width; c++){
        for(unsigned r = 0; r < img->height; r++){ // reads columns
            if(*get_bit(img, r, c) == 1){
                score++;
                update_max_score(score, &max_score, end, r, c);
            }
            else{
                score = 0; // breaks the line
            }
        }
        score = 0; //resets the score for a new column
    }

    // max_score is used as an offset for the first occurrence of 1
    start->row = end->row - max_score + 1;
    // start has to be in the same column as the end since this is a vertical line
    start->col = end->col;
    
    return max_score;
}


unsigned find_horizontal_line(Image* img, Coords* start, Coords* end){
    /*
    finds the largest horizontal line and writes its 
    coordinates into the start and end parameters,
    returns the length of the line
    */

    unsigned max_score = 0;
    unsigned score = 0;

    for(unsigned r = 0; r < img->height; r++){
        for(unsigned c = 0; c < img->width; c++){ // reads rows
            if(*get_bit(img, r, c) == 1){
                score++;
                update_max_score(score, &max_score, end, r, c);
            } else {
                score = 0; // breaks the line
            }
        }
        score = 0; //resets the score for a new column
    }
    
    // start has to be in the same row as the end since this is a horizontal line
    start->row = end->row;
    // max_score is used as an offset for the first occurrence of 1
    start->col = end->col - max_score + 1;

    return max_score;
}

unsigned find_square(Image* img, Coords* start, Coords* end){
    /*
    finds the largest square and writes its top left and bottom right
    coordinates into the start and end parameters,
    returns the length of square sides,
    if the square shape is valid it returns prematurely,
    */

    //finds out the largest possible square side
    unsigned biggest_square = min(img->height, img->width);
    unsigned max_score = 0;

    for(int dist = biggest_square; dist >= 0; dist--){
        //searches only the bits where the squares of size(dist) would fit
        for(unsigned r = 0; r < img->height - dist; r++){
            for(unsigned c = 0; c < img->width - dist; c++){
                //if the current bit is 1 and either theres a square or the square is just a point
                if(*get_bit(img, r, c) == 1 && (points_exist(img, r, c, dist) || dist == 0)){
                    update_max_score(dist + 1, &max_score, start, r, c);
                    end->row = start->row + max_score - 1; //has to be updated here because of the return
                    end->col = start->col + max_score - 1;
                    return max_score;
                }
            }
        }
    }
    return max_score;
}

bool points_exist(Image* img, unsigned row, unsigned col, unsigned distance){
    /*
    checks if points exist so that when connected create a square shape with a given distance,
    if it finds them it passes the information to the are_lines_valid function,
    returns if the square exists
    */

    //we can safely dereference here since we have prevented going out of bounds in find_square
    bit top_right = *get_bit(img, row, col + distance);
    bit bottom_left = *get_bit(img, row + distance, col);
    bit bottom_right = *get_bit(img, row + distance, col + distance);
    
    if(top_right && bottom_left && bottom_right){
        return are_lines_valid(img, row, col, distance);
    }
    return false;
}

bool are_lines_valid(Image* img, unsigned row, unsigned col, unsigned size){
    /*
    checks every line by checking if the corners are connected (make lines),
    returns true if all lines exist else returns false
    */
    
    for(unsigned i = 0; i < size; i++){
        if(*get_bit(img, row + i, col) != 1 // left
        || *get_bit(img, row, col + i) != 1 // top
        || *get_bit(img, row + i, col + size) != 1 // right
        || *get_bit(img, row + size, col + i) != 1){ // bottom
            return false;
        }
    }
    return true;
}

void update_max_score(unsigned score, unsigned* max_score, Coords* point, unsigned row, unsigned col){
    /*
    updates the max_score and the point coordinates to the given coordinates (row and col),
    if theres a new shape with the same size it compares their coords
    to see which one is the closest one based on the row,
    if the row is the same then it checks the column
    */

    if(score > *max_score){
        point->row = row;
        point->col = col;
        *max_score = score;
    }
    else if(score == *max_score && (point->row > row || (point->row == row && point->col > col))){
        point->row = row;
        point->col = col;
    }
}