// This is the CPP file you will edit and turn in.
// Also remove these comments here and add your own.
// TODO: rewrite this comment

#include <iostream>
#include "console.h"
#include "gwindow.h"
#include "grid.h"
#include "simpio.h"
#include "strlib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "math.h" //for sqrt and exp in the optional Gaussian kernel
#include "gmath.h" // for sinDegrees(), cosDegrees(), PI
using namespace std;

static const int    WHITE = 0xFFFFFF;
static const int    BLACK = 0x000000;
static const int    GREEN = 0x00FF00;

void     doFauxtoshop(GWindow &gw, GBufferedImage &img);

bool     openImageFromFilename(GBufferedImage& img, string filename);
bool 	 saveImageToFilename(const GBufferedImage &img, string filename);
void     getMouseClickLocation(int &row, int &col);
void     menu();
bool     isNum(string line);
void     scatter(Grid<int> &board, GBufferedImage &img);
void     edgeDetection(Grid<int> &board, GBufferedImage &img);
void     greenScreen(Grid<int> &board, GBufferedImage &img);
void     compareImg(GBufferedImage &img);
void     rotateImg(Grid<int> &board, GBufferedImage &img);
void     gaussianBlur(Grid<int> &board, GBufferedImage &img);
int      createRGB(int red, int green, int blue);
Vector<double> gaussKernelForRadius(int radius);

/* STARTER CODE FUNCTION - DO NOT EDIT
 *
 * This main simply declares a GWindow and a GBufferedImage for use
 * throughout the program. By asking you not to edit this function,
 * we are enforcing that the GWindow have a lifespan that spans the
 * entire duration of execution (trying to have more than one GWindow,
 * and/or GWindow(s) that go in and out of scope, can cause program
 * crashes).
 */
int main() {
    GWindow gw;
    gw.setTitle("Fauxtoshop");
    gw.setVisible(true);
    GBufferedImage img;
    doFauxtoshop(gw, img);
    return 0;
}

/* This is yours to edit. Depending on how you approach your problem
 * decomposition, you will want to rewrite some of these lines, move
 * them inside loops, or move them inside helper functions, etc.
 *
 * TODO: rewrite this comment.
 */
void doFauxtoshop(GWindow &gw, GBufferedImage &img) {
    string line;
    cout << "Welcome to Fauxtoshop!" << endl;
    while(true){
        line = getLine("Enter the name of an image file to open (or blank to quit)\n");
        while(!(openImageFromFilename(img, line)||line=="")){
            line = getLine("Invalid file name. Please input a valid file name\n");
        }
        if(line==""){
            gw.clear();
            gw.close();
            return;
        }
        gw.setCanvasSize(img.getWidth(), img.getHeight());
        gw.add(&img,0,0);
        Grid<int> board = img.toGrid();
        bool exit = isNum(line);
        while(!exit){
            menu();
            line = getLine("your choice:\n");
            if(isNum(line)){
                switch(stoi(line)){
                case 1:
                    scatter(board, img);
                    exit=true;
                    break;
                case 2:
                    edgeDetection(board, img);
                    exit=true;
                    break;
                case 3:
                    greenScreen(board, img);
                    exit=true;
                    break;
                case 4:
                    compareImg(img);
                    exit=true;
                    break;
                case 5:
                    rotateImg(board, img);
                    exit=true;
                    break;
                case 6:
                    gaussianBlur(board, img);
                    exit=true;
                    break;
                }
            }
        }
        line = getLine("Enter the filename to save image (or leave blank to skip saving)\n");
        while(!(saveImageToFilename(img, line)||line=="")){
            line = getLine("Invalid file name. Please input a valid file name\n");
        }
        gw.clear();
        cout<<endl;
    }
}

int difference(int pixel1, int pixel2){
    int redDifference, greenDifference, blueDifference, red1, red2, green1, green2, blue1, blue2;
    GBufferedImage::getRedGreenBlue(pixel1, red1, green1, blue1);
    GBufferedImage::getRedGreenBlue(pixel2, red2, green2, blue2);
    redDifference = abs(red1-red2);
    greenDifference = abs(green1-green2);
    blueDifference = abs(blue1-blue2);
    return max(max(redDifference, greenDifference), blueDifference);
}

void scatter(Grid<int> &board, GBufferedImage &img){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int degree;
    do{
        degree = getInteger("Enter degree of scatter [1-100]:\n", "Enter degree of scatter [1-100]:\n");
    }while(!(degree>=0&&degree<=100));
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            int ranRow = randomInteger(max(0,r-degree), min(board.numRows()-1,r+degree));
            int ranCol = randomInteger(max(0,c-degree), min(board.numCols()-1,c+degree));
            tempBoard[r][c] = board[ranRow][ranCol];
        }
    }
    board=tempBoard;
    img.fromGrid(board);
}

void edgeDetection(Grid<int> &board, GBufferedImage &img){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int threshold;
    do{
        threshold = getInteger("Enter threshold for edge detection:\n", "Enter threshold for edge detection:\n");
    }while(!(threshold>0));
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            bool edge=false;
            for(int i=r-1; i< (r+1); i++){
                for(int j=c-1; j<c+1; j++){
                    if(board.inBounds(i, j)){
                        if(difference(board[r][c], board[i][j])>threshold){
                            edge=true;
                        }
                    }
                }
            }
            if(edge){
                tempBoard[r][c]=BLACK;
            }
            else{
                tempBoard[r][c]=WHITE;
            }
        }
    }
    board=tempBoard;
    img.fromGrid(board);
}

void greenScreen(Grid<int> &background, GBufferedImage &img1){
    string line;
    GBufferedImage img2;
    int row, col;
    do{
        line = getLine("Now choose another file to add to your background image\nEnter name of image file to open:\n");
    }while(!(openImageFromFilename(img2, line)));
    openImageFromFilename(img2, line);
    Grid<int> foreground = img2.toGrid();
    int threshold;
    do{
        threshold = getInteger("Now choose a tolerence threshold:\n", "Now choose a tolerence threshold:\n");
    }while(!(threshold>0&&threshold<101));
    bool valid = false;
    while(!valid){
        line = getLine("Enter location to place image as \"(row,col)\" (or blank to use mouse):\n");
        if(line==""){
            getMouseClickLocation(row, col);
            valid=true;
        }
        else{
            stringstream split(line.substr(1, line.length()-2));
            string temp1, temp2;
            getline(split, temp1, ',');
            getline(split, temp2, ',');
            if(isNum(temp1)&&isNum(temp2)){
                valid=true;
                row=stoi(temp1);
                col=stoi(temp2);
            }
        }
    }

    for(int r=0; r<min(foreground.numRows(), background.numRows()-row); r++){
        for(int c=0; c<min(foreground.numCols(), background.numCols()-col); c++){
            if(difference(foreground[r][c], GREEN)>threshold){
                background[r+row][c+col]=foreground[r][c];
            }
        }
    }
    img1.fromGrid(background);
}

void compareImg(GBufferedImage &img1){
    string line;
    GBufferedImage img2;
    do{
        line = getLine("Now choose another file to add to your background image\nEnter name of image file to open:\n");
    }while(!(openImageFromFilename(img2, line)));
    openImageFromFilename(img2, line);
    int difference = img1.countDiffPixels(img2);
    if(difference==0){
        cout<<"These images are the same!"<<endl;
    }
    else{
        cout<<"These images differ in "<<difference<<" pixel locations!"<<endl;
    }
}

void rotateImg(Grid<int> &board, GBufferedImage &img){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int degree = getInteger("Enter an angle in degrees:\n", "Enter an angle in degrees:\n");
    cout<<sinDegrees(60)<<endl;
    cout<<cosDegrees(60)<<endl;

    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            tempBoard[r][c]=WHITE;
        }
    }
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            int xOld = (c - (board.numCols()/2)) * cosDegrees(degree) + (r - (board.numRows()/2)) * sinDegrees(degree);
            int yOld = -(c - (board.numCols()/2)) * sinDegrees(degree) + (r - (board.numRows()/2)) * cosDegrees(degree);
            if(board.inBounds(yOld+board.numRows()/2, xOld+board.numCols()/2)){
                tempBoard[r][c]=board[yOld+board.numRows()/2][xOld+board.numCols()/2];
            }
        }
    }
    board=tempBoard;
    img.fromGrid(board);
}

void gaussianBlur(Grid<int> &board, GBufferedImage &img){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int radius;
    do{
        radius = getInteger("Enter radius for gaussian blur that is greater than 0:\n", "Enter radius for gaussian blur that is greater than 0:\n");
    }while(!(radius>0));
    Vector<double> kernel = gaussKernelForRadius(radius);
    double sum=0;
    for(int i=0; i<kernel.size(); i++){
        sum+=kernel.get(i);
    }
    for(int i=0; i<kernel.size(); i++){
        kernel.set(i, kernel.get(i)/sum);
    }
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            double redSum1=0;
            double greenSum1=0;
            double blueSum1=0;
            for(int i=max(0,c-radius); i<=min(board.numCols()-1, c+radius); i++){
                int red, green, blue;
                sum++;
                GBufferedImage::getRedGreenBlue(board[r][i], red, green, blue);
                redSum1+=((double)red)*kernel.get(i-max(0,c-radius));
                greenSum1+=((double)green)*kernel.get(i-max(0,c-radius));
                blueSum1+=((double)blue)*kernel.get(i-max(0,c-radius));
            }
            tempBoard[r][c]=createRGB(redSum1, greenSum1, blueSum1);
        }
    }
    board=tempBoard;
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            double redSum2=0;
            double greenSum2=0;
            double blueSum2=0;
            for(int i=max(0,r-radius); i<=min(board.numRows()-1, r+radius); i++){
                int red, green, blue;
                GBufferedImage::getRedGreenBlue(board[i][c], red, green, blue);
                redSum2+=((double)red)*kernel.get(i-max(0,r-radius));
                greenSum2+=((double)green)*kernel.get(i-max(0,r-radius));
                blueSum2+=((double)blue)*kernel.get(i-max(0,r-radius));
            }
            tempBoard[r][c]=createRGB(redSum2, greenSum2, blueSum2);
        }
    }
    board=tempBoard;
    img.fromGrid(board);
}

int createRGB(int red, int green, int blue){
    return ((red & 0xff) << 16) + ((green & 0xff) << 8) + (blue & 0xff);
}

void menu(){
    cout<<"opening image file. May take a while\n"<<endl;
    cout<<"which image filter would you like to apply?\n"<<endl;
    cout<<"\t1 - scatter"<<endl;
    cout<<"\t2 - edge detection"<<endl;
    cout<<"\t3 - green screen with another image"<<endl;
    cout<<"\t4 - compare image with another image"<<endl;
    cout<<"\t5 - rotation"<<endl;
    cout<<"\t6 - gaussian blur"<<endl;
}

bool isNum(string line){
    for(char c:line){
        if(isdigit(c)==0){
            return false;
        }
    }
    return true;
}




/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to open the image file 'filename'.
 *
 * This function returns true when the image file was successfully
 * opened and the 'img' object now contains that image, otherwise it
 * returns false.
 */
bool openImageFromFilename(GBufferedImage& img, string filename) {
    try { img.load(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to save the image file to 'filename'.
 *
 * This function returns true when the image was successfully saved
 * to the file specified, otherwise it returns false.
 */
bool saveImageToFilename(const GBufferedImage &img, string filename) {
    try { img.save(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Waits for a mouse click in the GWindow and reports click location.
 *
 * When this function returns, row and col are set to the row and
 * column where a mouse click was detected.
 */
void getMouseClickLocation(int &row, int &col) {
    GMouseEvent me;
    do {
        me = getNextEvent(MOUSE_EVENT);
    } while (me.getEventType() != MOUSE_CLICKED);
    row = me.getY();
    col = me.getX();
}

/* HELPER FUNCTION
 *
 * This is a helper function for the Gaussian blur option.
 *
 * The function takes a radius and computes a 1-dimensional Gaussian blur kernel
 * with that radius. The 1-dimensional kernel can be applied to a
 * 2-dimensional image in two separate passes: first pass goes over
 * each row and does the horizontal convolutions, second pass goes
 * over each column and does the vertical convolutions. This is more
 * efficient than creating a 2-dimensional kernel and applying it in
 * one convolution pass.
 *
 * This code is based on the C# code posted by Stack Overflow user
 * "Cecil has a name" at this link:
 * http://stackoverflow.com/questions/1696113/how-do-i-gaussian-blur-an-image-without-using-any-in-built-gaussian-functions
 *
 */
Vector<double> gaussKernelForRadius(int radius) {
    if (radius < 1) {
        Vector<double> empty;
        return empty;
    }
    Vector<double> kernel(radius * 2 + 1);
    double magic1 = 1.0 / (2.0 * radius * radius);
    double magic2 = 1.0 / (sqrt(2.0 * PI) * radius);
    int r = -radius;
    double div = 0.0;
    for (int i = 0; i < kernel.size(); i++) {
        double x = r * r;
        kernel[i] = magic2 * exp(-x * magic1);
        r++;
        div += kernel[i];
    }
    for (int i = 0; i < kernel.size(); i++) {
        kernel[i] /= div;
    }
    return kernel;
}
