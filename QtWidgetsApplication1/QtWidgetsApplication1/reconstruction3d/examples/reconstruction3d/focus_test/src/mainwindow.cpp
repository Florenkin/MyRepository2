#include "mainwindow.h"
#include "ui_mainwindow.h"

void PopulateBinaryCodePatternData(std::vector<uchar> &Data, uint32_t Index, bool IsInvert)
{
    uint32_t Length = uint32_t(Data.size());
    uint32_t  MaxIndex  = uint8_t(ceil(log2(Length)));
    if (Index > MaxIndex) {
        return;
    }
    uint32_t TotalWidth= uint16_t(pow(2,MaxIndex));
    uint32_t BarWidth  = uint16_t(TotalWidth / pow(2,Index));
    uint32_t Offset    = 0/*(TotalWidth-Length)/2*/;
    uint8_t  PixelData = 0;
    uint8_t  PrePixelData = 0;

    bool reverseInclude = true;
    if (reverseInclude) {
        uint32_t PixelPos = Length;
        for (; PixelPos > 0; ) {
            PixelPos--;
            PixelData = (Offset/BarWidth)%2 ? 1 : 0;
            if (Index > 1) {
                PrePixelData = (Offset/(BarWidth*2))%2 ? 1 : 0;
                PixelData = PrePixelData ^ PixelData;
            }
            if (IsInvert) {
                PixelData = !PixelData;
            }
            Data[PixelPos] = PixelData*200;
            Offset++;
        }
    }else {
        uint32_t PixelPos  = 0;
        for (; PixelPos < Length; PixelPos++) {
            PixelData = (Offset/BarWidth)%2 ? 1 : 0;
            if (Index > 1) {
                PrePixelData = (Offset/(BarWidth*2))%2 ? 1 : 0;
                PixelData = PrePixelData ^ PixelData;
            }
            if (IsInvert) {
                PixelData = !PixelData;
            }
            Data[PixelPos] = PixelData*200;
            Offset++;
        }
    }
}

void PopulatePhasePatternData(std::vector<uchar> &Data, uint32_t PixelPerPhase, uint32_t Index, uint32_t NumPhaseStep)
{
    uint32_t Length = uint32_t(Data.size());
    if (Index > NumPhaseStep) {
        return;
    }

    double   PhaseStep    = CV_PI*2/PixelPerPhase;
    double   PhaseOffset  = CV_PI *2/NumPhaseStep;
    double   PhaseStart   = -CV_PI;
    uint8_t  PixelData = 0;

    bool reverseInclude = true;
    if (reverseInclude) {
        uint32_t PixelPos = Length;
        uint32_t PixelRealPos = 0;
        for (; PixelPos > 0;) {
            PixelPos--;
            PixelData = uint8_t(200*((cos(PhaseStart + (PixelRealPos%PixelPerPhase)*PhaseStep - PhaseOffset*(Index-1))+1)/2));
            Data[PixelPos] = PixelData;
            PixelRealPos++;
        }
    } else {
        uint16_t PixelPos = 0;
        for (; PixelPos < Length; PixelPos++) {
            PixelData = uint8_t(200*((cos(PhaseStart + (PixelPos%PixelPerPhase)*PhaseStep - PhaseOffset*(Index-1))+1)/2));
            Data[PixelPos] = PixelData;
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    for (uint32_t picI=0; picI < src_image_vec.size(); picI++) {
        for (uint32_t picJ=0; picJ < src_image_vec[picI].size(); picJ++) {
            delete src_image_vec[picI][picJ];
        }
    }

    delete ui;
}