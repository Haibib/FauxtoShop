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
void     scatter(Grid<int> &board);
void     edgeDetection(Grid<int> &board);
void     cannyEdgeDetection(Grid<int> &board);
void     euclideanEdgeDetection(Grid<int> &board);
void     greenScreen(Grid<int> &board, string &line);
void     compareImg(GBufferedImage &img1, string &line);
void     rotateImg(Grid<int> &board);
void     gaussianBlur(Grid<int> &board, int radius);
void     getSecondFile(GBufferedImage &img2, string &line);
int      createRGB(int red, int green, int blue);
int      euclideanDifference(int pixel1, int pixel2);
int      difference(int pixel1, int pixel2);
void     directionalConvolution(Grid<int> &board, Grid<int> &tempBoard, int radius, Vector<double> &kernel, bool horizontal);
void     grayScale(Grid<int> &board);
void     grayScaleSimplify(Grid<int> &board);
void     nonMaximumSuppression(Grid<int> &tempBoard, Vector<int> &theta);
void     matrixConvolution(Grid<int> &board, Grid<int> &tempBoard, Grid<double>&kernel, int radius);
void     doubleThreshold(Grid<int> &tempBoard, int highThreshold);
void     hysteresis(Grid<int> &board, Grid<int> &tempBoard);
bool     validHex(string &line);
void     draw(Grid<int> &board, GBufferedImage& img, string &line, GWindow &gw);
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
        img.sendToBack();
        Grid<int> board = img.toGrid();
        bool exit = isNum(line);
        while(!exit){
            menu();
            line = getLine("your choice:\n");
            if(isNum(line)){
                switch(stoi(line)){
                case 1:
                    scatter(board);
                    exit=true;
                    break;
                case 2:
                    edgeDetection(board);
                    exit=true;
                    break;
                case 3:
                    greenScreen(board, line);
                    exit=true;
                    break;
                case 4:
                    compareImg(img, line);
                    exit=true;
                    break;
                case 5:
                    rotateImg(board);
                    exit=true;
                    break;
                case 6:
                    //Negative radius by default to prompt user
                    gaussianBlur(board, -1);
                    exit=true;
                    break;
                case 7:
                    euclideanEdgeDetection(board);
                    exit=true;
                    break;
                case 8:
                    cannyEdgeDetection(board);
                    exit=true;
                    break;
                case 9:
                    grayScale(board);
                    exit=true;
                    break;
                case 10:
                    img.sendToBack();
                    draw(board, img, line, gw);
                    exit=true;
                    break;
                }
            }
        }
        img.fromGrid(board);
        img.sendToBack();
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
//Function that calculates the euclidean distance between two pixels
int euclideanDifference(int pixel1, int pixel2){
    int red1, red2, green1, green2, blue1, blue2;
    GBufferedImage::getRedGreenBlue(pixel1, red1, green1, blue1);
    GBufferedImage::getRedGreenBlue(pixel2, red2, green2, blue2);
    //Formula for Euclidean distance between two three-dimensional vectors
    return sqrt(pow((red1-red2),2) + pow((green1-green2),2) + pow((blue1-blue2),2));
}

void getSecondFile(GBufferedImage &img2, string &line){
    do{
        line = getLine("Now choose another file to add to your background image\nEnter name of image file to open:\n");
    }while(!(openImageFromFilename(img2, line)));
    openImageFromFilename(img2, line);
}

void scatter(Grid<int> &board){
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
}

void edgeDetection(Grid<int> &board){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int threshold;
    do{
        threshold = getInteger("Enter threshold for edge detection:\n", "Enter threshold for edge detection:\n");
    }while(!(threshold>0));
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            tempBoard[r][c]=WHITE;
            for(int i=r-1; i< (r+1); i++){
                for(int j=c-1; j<c+1; j++){
                    if(board.inBounds(i, j)){
                        if(difference(board[r][c], board[i][j])>threshold){
                            tempBoard[r][c]=BLACK;
                        }
                    }
                }
            }
        }
    }
    board=tempBoard;
}

void euclideanEdgeDetection(Grid<int> &board){
    gaussianBlur(board, 1);
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int threshold;
    do{
        threshold = getInteger("Enter threshold for edge detection:\n", "Enter threshold for edge detection:\n");
    }while(!(threshold>0));
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            tempBoard[r][c]=WHITE;
            for(int i=r-1; i< (r+1); i++){
                for(int j=c-1; j<c+1; j++){
                    if(board.inBounds(i, j)){
                        if(euclideanDifference(board[r][c], board[i][j])>threshold){
                            tempBoard[r][c]=BLACK;
                        }
                    }
                }
            }
        }
    }
    board=tempBoard;
}

void matrixConvolution(Grid<int> &board, Grid<int> &tempBoard, Grid<double>&kernel, int radius){
    for(int rb=1; rb<board.numRows()-1; rb++){
        for(int cb=1; cb<board.numCols()-1; cb++){
            int temp=0;
            for(int rk=-radius; rk<=radius; rk++){
                for(int ck=-radius; ck<=radius; ck++){
                    temp+=(board[rb+rk][cb+ck]*kernel[rk+radius][ck+radius]);
                }
            }
            tempBoard[rb][cb]=temp/8;
        }
    }
}

void cannyEdgeDetection(Grid<int> &board){
    //gaussianBlur(board, 1);
    grayScaleSimplify(board);
    double tempHorizontalKernel[]={1, 0, -1, 2, 0, -2, 1, 0, -1};
    double tempVerticalKernel[]={1, 2, 1, 0, 0, 0, -1, -2, -1};
    Grid<double> horizontalKernel(3,3);
    Grid<double> verticalKernel(3,3);
    int count=0;
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            horizontalKernel[i][j]=tempHorizontalKernel[count];
            verticalKernel[i][j]=tempVerticalKernel[count];
            count++;
        }
    }
    //Intensity gradient in the x-direction
    Grid<int>tempBoardX(board.numRows(), board.numCols());
    matrixConvolution(board, tempBoardX, horizontalKernel, 1);
    //Intensity gradient in the y-direction
    Grid<int>tempBoardY(board.numRows(), board.numCols());
    matrixConvolution(board, tempBoardY, verticalKernel, 1);
    //Stores the gradient magnitudes
    Grid<int>tempBoard(board.numRows(), board.numCols());
    //Stores the gradient angles
    Vector<int>theta;
    int highThreshold=0;
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            board[r][c]=BLACK;
            tempBoard[r][c]=sqrt(pow(tempBoardX[r][c],2)+pow(tempBoardY[r][c],2));
            theta.add(round(toDegrees(atan2(tempBoardY[r][c],tempBoardX[r][c]))/45.0));
            if(tempBoard[r][c]>highThreshold){
                highThreshold=tempBoard[r][c];
            }
        }
    }
    nonMaximumSuppression(tempBoard, theta);
    doubleThreshold(tempBoard, highThreshold);
    hysteresis(board, tempBoard);
}


void hysteresis(Grid<int> &board, Grid<int> &tempBoard){
    for(int r=1; r<board.numRows()-1; r++){
        for(int c=1; c<board.numCols()-1; c++){
            bool strong=false;
            if(tempBoard[r][c]!=0){
                for(int i=-1; i<=1; i++){
                    for(int j=-1; j<=1; j++){
                        if(tempBoard[r+i][c+j]==255){
                            strong=true;
                        }
                    }
                }
            }
            if(strong==true){
                board[r][c]=WHITE;
            }
        }
    }
}

void doubleThreshold(Grid<int> &tempBoard, int highThreshold){
    for(int r=0; r<tempBoard.numRows(); r++){
        for(int c=0; c<tempBoard.numCols(); c++){
            if(tempBoard[r][c]>(highThreshold*0.09)){
                tempBoard[r][c]=255;
            }
            else if(tempBoard[r][c]<(highThreshold*0.05)){
                tempBoard[r][c]=0;
            }
            else{
                tempBoard[r][c]=100;
            }
        }
    }
}

void nonMaximumSuppression(Grid<int> &tempBoard, Vector<int> &theta){
    int i=0;
    for(int r=1; r<tempBoard.numRows()-1; r++){
        for(int c=1; c<tempBoard.numCols()-1; c++){
            if(theta.get(i)==0||theta.get(i)==4||theta.get(i)==-4){
                if(tempBoard[r][c]<tempBoard[r][c+1]||tempBoard[r][c]<tempBoard[r][c-1]){
                    tempBoard[r][c]=0;
                }
            }
            else if(theta.get(i)==-3||theta.get(i)==1){
                if(tempBoard[r][c]<tempBoard[r+1][c+1]||tempBoard[r][c]<tempBoard[r-1][c-1]){
                    tempBoard[r][c]=0;
                }
            }
            else if(theta.get(i)==2||theta.get(i)==-2){
                if(tempBoard[r][c]<tempBoard[r+1][c]||tempBoard[r][c]<tempBoard[r-1][c]){
                    tempBoard[r][c]=0;
                }
            }
            else if(theta.get(i)==-1||theta.get(i)==3){
                if(tempBoard[r][c]<tempBoard[r-1][c+1]||tempBoard[r][c]<tempBoard[r+1][c-1]){
                    tempBoard[r][c]=0;
                }
            }
            i++;
        }
    }
}

void grayScaleSimplify(Grid<int> &board){
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            int red, green, blue;
            GBufferedImage::getRedGreenBlue(board[r][c], red, green, blue);
            int average = (red+green+blue)/3;
            board[r][c]=average;
            //cout<<board[r][c]<<endl;
        }
    }
}

void grayScale(Grid<int> &board){
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            int red, green, blue;
            GBufferedImage::getRedGreenBlue(board[r][c], red, green, blue);
            int average = (red+green+blue)/3;
            board[r][c]=createRGB(average, average, average);
        }
    }
}

void greenScreen(Grid<int> &background, string &line){
    GBufferedImage img2;
    getSecondFile(img2, line);
    int row, col;
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
}

void compareImg(GBufferedImage &img1, string &line){
    GBufferedImage img2;
    getSecondFile(img2, line);
    int difference = img1.countDiffPixels(img2);
    if(difference==0){
        cout<<"These images are the same!"<<endl;
    }
    else{
        cout<<"These images differ in "<<difference<<" pixel locations!"<<endl;
    }
}

void rotateImg(Grid<int> &board){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    int degree = getInteger("Enter an angle in degrees:\n", "Enter an angle in degrees:\n");
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
}

void directionalConvolution(Grid<int> &board, Grid<int> &tempBoard, int radius, Vector<double> &kernel, bool horizontal){
    int i, end, direction;
    for(int r=0; r<board.numRows(); r++){
        for(int c=0; c<board.numCols(); c++){
            if(horizontal){
                direction=c;
                i=max(0,direction-radius);
                end=min(board.numCols()-1, c+radius);
            }
            else{
                direction=r;
                i=max(0,r-radius);
                end=min(board.numRows()-1, direction+radius);
            }
            double redSum=0, greenSum=0, blueSum=0;
            while(i<=end){
                int red, green, blue;
                if(horizontal){
                    GBufferedImage::getRedGreenBlue(board[r][i], red, green, blue);
                }
                else{
                    GBufferedImage::getRedGreenBlue(board[i][c], red, green, blue);
                }
                redSum+=((double)red)*kernel.get(i-max(0,direction-radius));
                greenSum+=((double)green)*kernel.get(i-max(0,direction-radius));
                blueSum+=((double)blue)*kernel.get(i-max(0,direction-radius));
                i++;
            }
            tempBoard[r][c]=createRGB(redSum, greenSum, blueSum);
        }
    }
}

void gaussianBlur(Grid<int> &board, int radius){
    Grid<int>tempBoard(board.numRows(), board.numCols());
    //Negative radius prompts user for an input
    if(radius==-1){
        do{
            radius = getInteger("Enter radius for gaussian blur that is greater than 0:\n", "Enter radius for gaussian blur that is greater than 0:\n");
        }while(!(radius>0));
    }
    Vector<double> kernel = gaussKernelForRadius(radius);
    //Normalizing the gaussian kernels (in case it isn't already normalized)
    double sum=0;
    for(int i=0; i<kernel.size(); i++){
        sum+=kernel.get(i);
    }
    for(int i=0; i<kernel.size(); i++){
        kernel.set(i, kernel.get(i)/sum);
    }
    directionalConvolution(board, tempBoard, radius, kernel, true);
    board=tempBoard;
    directionalConvolution(board, tempBoard, radius, kernel, false);
    board=tempBoard;
}

void draw(Grid<int> &board, GBufferedImage &img, string &line, GWindow &gw){
    do{
        line=getLine("Enter a valid hexidecimal color starting with the \'#\' sign\n");
    }while(!(validHex(line)));
    int color = convertColorToRGB(line);
    cout<<"draw anything press \'q\' to exit"<<endl;
    GEvent e;
    bool exit=false;
    do{
        e = waitForEvent(MOUSE_EVENT+KEY_EVENT);
        switch (e.getEventClass()) {
            case MOUSE_EVENT:{
                GMouseEvent mouseE(e);
                if (mouseE.getEventType() == MOUSE_PRESSED||mouseE.getEventType() == MOUSE_DRAGGED) {
                    board[mouseE.getY()][mouseE.getX()]=color;
                }
                img.fromGrid(board);
                break;
            }
            case KEY_EVENT:{
                GKeyEvent keyE(e);
                if(keyE.getKeyChar()=='q'){
                    exit=true;
                }
                break;
            }
        }
    }while(!exit);
}

bool validHex(string &line){
    if (line[0] != '#'){
        return false;
    }
    if (!(line.length() == 4 or line.length() == 7)){
        return false;
    }
    for (int i = 1; i < line.length(); i++){
        if (!((line[i] >= '0' && line[i] <= 9) || (line[i] >= 'a' && line[i] <= 'f') || (line[i] >= 'A' || line[i] <= 'F'))){
            return false;
        }
    }
    return true;
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
    cout<<"\t7 - euclidean edge detection"<<endl;
    cout<<"\t8 - canny edge detection"<<endl;
    cout<<"\t9 - grayscale"<<endl;
    cout<<"\t10 - draw"<<endl;
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
