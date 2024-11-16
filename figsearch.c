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


Image* image_ctor(int** pixels, unsigned int width, unsigned int height);

void image_dtor(Image* img);


int arg_parse(int count, char** args);

void print_help();

int test_file_structure(char* file_name);

int find_vertical_line();

int find_horizontal_line();

int find_square();

int validate_size(FILE* file, unsigned* first, unsigned* second);

int validate_bitmap(FILE* file, unsigned width, unsigned height);


int main(int argc, char** argv){

    int err = arg_parse(argc - 1, argv);
    if(err) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

Image* image_ctor(int** pixels, unsigned width, unsigned height){
    Image* img = malloc(sizeof(Image));
    if(img == NULL) return NULL;
    
    img->height = height;
    img->width = width;

    int* data = malloc(sizeof(int) * width * height);
    if(data == NULL) return NULL;
    
    for(unsigned i = 0; i < height; i++){
        for(unsigned j = 0; j < width; j++){
            data[(i * img->height) + j] = pixels[i][j];
        }
    }
    img->data = data;
    return img;
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
        int res = test_file_structure(args[2]);
        if(res == EXIT_FAILURE) return EXIT_FAILURE;
    }
    else if(count == 2 && strcmp(args[1], "hline") == 0){
        //find_horizontal_line();
    }
    else if(count == 2 && strcmp(args[1], "vline") == 0){
        //find_vertical_line();
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

int test_file_structure(char* file_name){
    FILE* file = fopen(file_name,"r");
    unsigned height, width;
    
    if(file == NULL){
        print_err("Failed to open the file");
        fclose(file);
        return EXIT_FAILURE;
    }
    if(validate_size(file, &height, &width) == EXIT_FAILURE 
    || validate_bitmap(file, width, height) == EXIT_FAILURE){
        print_err("Invalid");
        fclose(file);
        return EXIT_FAILURE;
    }
    printf("Valid\n");
    fclose(file);
    return EXIT_SUCCESS;
}

int validate_size(FILE* file, unsigned* first, unsigned* second){
    char buff[MAX_LINE_LEN];
    char* next; // points at the expected next number
    char* end; // points at the end of the string (end of the line)
    
    if(fgets(buff, sizeof(buff), file) == NULL){ 
        return EXIT_FAILURE;
    }
    *first = strtol(buff, &next, 10);
    if(*next != ' '){
        return EXIT_FAILURE;
    }
    next++;
    *second = strtol(next, &end, 10);
    if(*first == 0 || *second == 0 || *end != '\r'){ //CHANGE THIS WHEN TESTING ON LINUX TO \n
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int validate_bitmap(FILE* file, unsigned width, unsigned height){
    char buffer[MAX_LINE_LEN];
    unsigned line_count = 0, bit_count = 0;

    while(fgets(buffer, sizeof(buffer), file) != NULL){
        for(unsigned i = 0;buffer[i] != '\r' && buffer[i] != '\0'; i++){ //CHANGE THIS WHEN TESTING ON LINUX TO \n
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
        line_count++;
    }

    if(line_count != height || bit_count != width * height){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}