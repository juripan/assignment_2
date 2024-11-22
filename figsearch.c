#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Image* load_image_data(char* file_name);

void image_dtor(Image* img);


int arg_parse(int count, char** args);

void print_help();

int test_file_structure(char* file_name);

int find_vertical_line(char* file_name, Coords* start, Coords* end);

int find_horizontal_line();

int find_square();

int get_size(FILE* file, unsigned* rows, unsigned* columns);

int validate_bitmap(FILE* file, unsigned width, unsigned height);


int main(int argc, char** argv){

    int err = arg_parse(argc - 1, argv);
    if(err) return EXIT_FAILURE;

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


int* get_bit(Image* img, unsigned row, unsigned col){
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
    if(count > 2){
        print_err("Too many of arguments");
        return EXIT_FAILURE;
    }
    if(count == 1 && strcmp(args[1], "--help") == 0){
        print_help();
    }
    else if(count == 2 && strcmp(args[1], "test") == 0){
        if(test_file_structure(args[2]) == EXIT_FAILURE)
            return EXIT_FAILURE;
        printf("Valid\n");
    }
    else if(count == 2 && strcmp(args[1], "hline") == 0){
        //Coords start = {0};
        //Coords end = {0};
        //if(test_file_structure(args[2]) == EXIT_FAILURE 
        //|| find_horizontal_line(args[2], &start, &end) == EXIT_FAILURE){
        //    return EXIT_FAILURE;
        //}
        //printf("start row %d col %d\n", start.row, start.col);
        //printf("end row %d col %d\n", end.row, end.col);
    }
    else if(count == 2 && strcmp(args[1], "vline") == 0){
        Coords start = {0};
        Coords end = {0};
        if(test_file_structure(args[2]) == EXIT_FAILURE 
        || find_vertical_line(args[2], &start, &end) == EXIT_FAILURE){
            return EXIT_FAILURE;
        }
        printf("start row %d col %d\n", start.row, start.col);
        printf("end row %d col %d\n", end.row, end.col);
    }
    else if(count == 2 && strcmp(args[1], "square") == 0){
        //find_square();
    }
    else{
        print_err("Incorrect arguments");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
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


int test_file_structure(char* file_name){ //TODO: make this fit the requirements
    /*
    opens a file with the file name that is passed in and checks if it has a valid file structure,
    returns EXIT_FAILURE if the test fails, else it returns EXIT_SUCCESS
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


int find_vertical_line(char* file_name, Coords* start, Coords* end){
    Image* img = load_image_data(file_name);
    if(img == NULL) return EXIT_FAILURE;

    unsigned max_score = 0;
    unsigned score = 0;

    for(unsigned c = 0; c < img->width; c++){
        for(unsigned r = 0; r < img->height; r++){ // reads columns
            if(*get_bit(img, r, c) == 1){
                score++;
                if(score > max_score){
                    start->row = r - score + 1; // score is used as an offset for the first occurrence of 1
                    start->col = c; // start has to be in the same column as the end since this is a vertical line
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
    return EXIT_SUCCESS;
}


int get_size(FILE* file, unsigned* rows, unsigned* columns){
    /*
    reads and validates the first line (size of bitmap),
    writes into the rows and columns addresses the result of parsing the size,
    returns EXIT_FAILURE if it fails,
    */
    char buff[MAX_LINE_LEN];
    char* next; // points at the expected next char
    char* end; // points at the end of the string (end of the line)
    
    if(fgets(buff, sizeof(buff), file) == NULL){ 
        return EXIT_FAILURE;
    }

    *rows = strtol(buff, &next, 10);

    if(*next != ' '){
        return EXIT_FAILURE;
    }
    next++;
    
    *columns = strtol(next, &end, 10);

    if(*rows == 0 || *columns == 0 || *end != '\r'){ //CHANGE THIS WHEN TESTING ON LINUX TO \n
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
    char buffer[MAX_LINE_LEN]; //TODO: maybe make it a malloc so the buffer is adjusted to its width
    unsigned row_count = 0, bit_count = 0;

    while(fgets(buffer, sizeof(buffer), file) != NULL){
        for(unsigned i = 0; buffer[i] != '\r' && buffer[i] != '\0'; i++){ //CHANGE THIS WHEN TESTING ON LINUX TO \n
            if(i % 2 == 0 && (buffer[i] == '1' || buffer[i] == '0')){
                bit_count++;
            }
            else if(i % 2 != 0 && buffer[i] == ' '){
                continue;
            }
            else{
                return EXIT_FAILURE;
            }
        }
        row_count++;
    }

    if(row_count != height || bit_count != width * height){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
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

    if(get_size(file, &col, &row) == EXIT_FAILURE){ //TODO: remove the check if redundant
        print_err("Invalid data");
        fclose(file);
        return NULL;
    }

    Image* img = image_ctor(col, row);

    if(img == NULL){
        print_err("Image allocation failed");
        fclose(file);
        return NULL;
    }

    char buffer[MAX_LINE_LEN];
    unsigned data_index = 0;

    while(fgets(buffer, sizeof(buffer), file) != NULL){
        for(unsigned i = 0; buffer[i] != '\r' && buffer[i] != '\0'; i++){ //CHANGE THIS WHEN TESTING ON LINUX TO \n
            if(i % 2 == 0 && (buffer[i] == '1' || buffer[i] == '0')){
                img->data[data_index++] = buffer[i] - '0'; // subtracting '0' converts the char into an int
            }
        }
    }
    fclose(file);
    return img;
}