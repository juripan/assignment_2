#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_LEN 101
#define print_err(msg) fprintf(stderr, msg "\n")


typedef struct{
    int* data;
    unsigned int width;
    unsigned int height;
}Image;

typedef struct{
    unsigned row;
    unsigned col;
}Coords;


Image* image_ctor(unsigned height, unsigned width);

Image* load_image_data(char* file_name); //TODO: maybe make this use an existing image instead of calling image_ctor

void image_dtor(Image* img);


int arg_parse(int count, char** args);

void print_help();

int test_command(char* file_name);

int test_file_structure(char* file_name);

int get_size(FILE* file, unsigned* rows, unsigned* columns);

int validate_bitmap(FILE* file, unsigned width, unsigned height);

int find_shape(char* file_name, unsigned find_func(Image*, Coords*, Coords*));

unsigned find_vertical_line(Image* img, Coords* start, Coords* end);

unsigned find_horizontal_line(Image* img, Coords* start, Coords* end);

int find_square(Image* img, Coords* start, Coords* end); //TODO: add this next


int main(int argc, char** argv){
    if(arg_parse(argc - 1, argv) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}


Image* image_ctor(unsigned height, unsigned width){
    Image* img = malloc(sizeof(Image));
    if(img == NULL) return NULL;

    int* data = malloc(sizeof(int) * width * height);
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
        if(buff == '1' || buff == '0'){
            img->data[data_index++] = buff - '0'; // subtracting '0' converts the char into an int
        }
    }
    fclose(file);
    return img;
}

int* get_bit(Image* img, unsigned row, unsigned col){
    /*
    returns NULL if the row or col are out of bounds,
    returns a pointer to a location specified by the row and col parameters
    */
    if(row > img->height || col > img->width){
        return NULL;
    }
    return &(img->data[(row * img->width) + col]);
}


void image_dtor(Image* img){
    free(img->data);
    free(img);
    img = NULL;
}


int arg_parse(int count, char** args){
    /*
    parses the passed in arguments and triggers the corresponding function,
    returns EXIT_FAILURE if anything goes wrong, otherwise retruns EXIT_SUCCESS 
    */
    if(count > 2){
        print_err("Too many of arguments");
        return EXIT_FAILURE;
    }
    if(count == 1 && strcmp(args[1], "--help") == 0){
        print_help();
        return EXIT_SUCCESS;
    }
    else if(count == 2 && strcmp(args[1], "test") == 0){
        return test_command(args[2]);
    }
    if(count == 2 && strcmp(args[1], "hline") == 0){
        return find_shape(args[2], find_horizontal_line);
    }
    if(count == 2 && strcmp(args[1], "vline") == 0){
        return find_shape(args[2], find_vertical_line);
    }
    if(count == 2 && strcmp(args[1], "square") == 0){
        //return find_shape(args[2], find_square);
    }
    print_err("Incorrect arguments");
    return EXIT_FAILURE;
}


void print_help(){
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
    char buff[MAX_LINE_LEN];
    char* next; // points at the expected next char
    char* end; // points at the end of the string (end of the line)
    int whitespace_count = 0;
    unsigned i;

    for(i = 0; (buff[i] = fgetc(file)) != EOF; i++){
        if(isspace(buff[i])!= 0){
            whitespace_count++;
            if(whitespace_count == 2){ //reads the first two numbers
                break;
            }
        }
    }
    buff[i+1] = '\0';

    *rows = strtol(buff, &next, 10);
    
    *columns = strtol(next, &end, 10);

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
        if(/*i % 2 == 0 &&*/ (buff == '1' || buff == '0')){ //UNCOMMENT THESE WHEN TESTING ON LINUX
            bit_count++;
        }
        else if(/*i % 2 != 0 &&*/ isspace(buff)){
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

    if(test_file_structure(file_name) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }

    Image* img = load_image_data(file_name);
    if(img == NULL) return EXIT_FAILURE;

    unsigned size = find_func(img, &start, &end);
    if(size == 0){
        printf("Not found\n");
        return EXIT_SUCCESS;
    }

    printf("start row %d col %d\n", start.row, start.col);
    printf("end row %d col %d\n", end.row, end.col);
    return EXIT_SUCCESS;
}


unsigned find_vertical_line(Image* img, Coords* start, Coords* end){
    /*
    function that finds the largest vertical line and writes its 
    coordinates into the start and end parameters,
    returns the length of the line
    */
    unsigned max_score = 0;
    unsigned score = 0;

    for(unsigned c = 0; c < img->width; c++){
        for(unsigned r = 0; r < img->height; r++){ // reads columns
            if(*get_bit(img, r, c) == 1){
                score++;
                if(score > max_score){
                    end->row = r;
                    end->col = c;
                    max_score = score;
                }
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
    function that finds the largest horizontal line and writes its 
    coordinates into the start and end parameters,
    returns the length of the line
    */
    unsigned max_score = 0;
    unsigned score = 0;

    for(unsigned r = 0; r < img->height; r++){
        for(unsigned c = 0; c < img->width; c++){ // reads rows
            if(*get_bit(img, r, c) == 1){
                score++;
                if(score > max_score){
                    end->row = r;
                    end->col = c;
                    max_score = score;
                }
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

/*
int find_square(Image* img, Coords* start, Coords* end){
    return EXIT_SUCCESS;
}
*/
