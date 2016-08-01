#include "opencv2\core.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;


string makeZeroLedString(int i)
{
    string zeros;
    if (i < 10) zeros = "000";
    else if (i < 100) zeros = "00";
    else if (i < 1000) zeros = "0";
    return zeros + to_string(i);
}

void main()
{
    string gdsPath, waveguidePath;
    
    cout << "Provide the filename of the full GDS image: ";
    cin >> gdsPath;

    cout << "Provide the filename of the waveguide only image: ";
    cin >> waveguidePath;

    Mat GDS = 255 - imread(gdsPath, IMREAD_GRAYSCALE);
    Mat waveguides = 255 - imread(waveguidePath, IMREAD_GRAYSCALE);

    double scaleFactor;
    cout << "Specify the scaling to be applied to the image to match the microscope images (0 if images are scaled manually): ";
    cin >> scaleFactor;

    resize(GDS, GDS, Size(), scaleFactor, scaleFactor);
    resize(waveguides, waveguides, Size(), scaleFactor, scaleFactor);

    int rows, cols;
    cout << "Number of rows: ";
    cin >> rows;
    cout << "Number of cols: ";
    cin >> cols;

    int tiles = rows * cols;

    string templateName;
    cout << "Provide the prefix name of the templates: ";
    cin >> templateName;

    cout << "If region images are desired, create \"Regions\" subfolder here" << endl;

    for (int t = 0; t < tiles; t++)
    {
        int r = t % (rows);
        int c = (t) / (rows);
        Mat red = imread(templateName + "_" + makeZeroLedString(t + 1) + "_template.png", IMREAD_GRAYSCALE);

        if (((t) / (rows)) % 2)
        {
            cout << "true" << endl;
            r = rows - 1 - r;
        }
        cout << t << " " << r << " " << c << endl;

        int cropPad = 1000;
        int cropRow = max(0, red.rows * (r));
        int cropCol = max(0, red.cols * (c));


        int dx = red.cols; if (cropCol + dx > GDS.cols) dx = GDS.cols - cropCol;
        int dy = red.rows; if (cropRow + dy > GDS.rows) dy = GDS.rows - cropRow;
        Mat croppedSection = GDS(Rect(cropCol, cropRow, dx, dy));
        Mat croppedWG = waveguides(Rect(cropCol, cropRow, dx, dy));

        cout << cropRow << " , " << cropCol  << endl;

        Mat result;
        copyMakeBorder(croppedSection, croppedSection, cropPad, cropPad, cropPad, cropPad, BORDER_CONSTANT);
        copyMakeBorder(croppedWG, croppedWG, cropPad, cropPad, cropPad, cropPad, BORDER_CONSTANT);
        matchTemplate(croppedSection, red, result, CV_TM_CCORR_NORMED);
        //matchTemplate(croppedSection, red, result, CV_TM_CCOEFF_NORMED);
        imwrite("Regions/" + templateName + "_" + makeZeroLedString(t+1) + "_region.png", croppedSection);


        Point min, max;
        minMaxLoc(result, NULL, NULL, &min, &max);
        Point match = max;

        Point fix = Point(0, 0);

        Mat matched = croppedSection(Rect(match + fix, match + Point(red.cols, red.rows)));
        Mat matchedWG = croppedWG(Rect(match + fix, match + Point(red.cols, red.rows)));
        imwrite(templateName + "_" + makeZeroLedString(t+1) + "_mask.png", matchedWG);
    }
}