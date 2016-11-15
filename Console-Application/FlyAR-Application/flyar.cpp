#include "flyar.h"

FlyAR::FlyAR(QWidget *parent) :
    QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    myPenWidth = 3;
    myPenColor = Qt::blue;
    outFileName = "../TestData/Data.txt";
    objCount = 0;
    objHeight = 1.0f;
    objType = 1;
    xRadius = 20.0f;
    yRadius = 20.0f;
}

bool FlyAR::openImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName))
        return false;

    QSize newSize = loadedImage.size().expandedTo(size());
    resizeImage(&loadedImage, newSize);
    image = loadedImage;
    modified = false;
    update();
    return true;
}

bool FlyAR::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    if (visibleImage.save(fileName, fileFormat)) {
        modified = false;
        writeToFile(); //write to the file when saves
        return true;
    } else {
        return false;
    }
}

void FlyAR::setPenColor(const QColor &newColor)
{
    myPenColor = newColor;
}

void FlyAR::setPenWidth(int newWidth)
{
    myPenWidth = newWidth;
}

void FlyAR::clearImage()
{
    image.fill(qRgb(255, 255, 255));
    objCount = 0; //reset the counter
    objVec.clear(); //empty the vector
    modified = true;
    update();
}

void FlyAR::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        bool ok, shape, xrad, yrad = false;
        lastPoint = event->pos();

        objType = QInputDialog::getInt(this, tr("FlyAR"),
                                            tr("1=Circle\n2=Square"),
                                            objType,
                                            1, 2, 1, &shape);
        if (shape)
        {
            objHeight = QInputDialog::getInt(this, tr("FlyAR"),
                                                tr("Give the object a height:"),
                                                objHeight,
                                                1.0f, 15.0f, 1.0f, &ok);
            if (ok)
            {
                xRadius = QInputDialog::getInt(this, tr("FlyAR"),
                                                    tr("X-Radius (10-50): "),
                                                    xRadius,
                                                    10, 50, 2, &xrad);
                if (xrad)
                {
                    yRadius = QInputDialog::getInt(this, tr("FlyAR"),
                                                        tr("Y-Radius (10-50): "),
                                                        yRadius,
                                                        10, 50, 2, &yrad);
                    if (yrad)
                    {
                        drawObj(event->pos());

                        objVec.push_back(obj());

                        objVec[objCount].type = objType;
                        objVec[objCount].r = (myPenColor.rgb() >> 16) & 0xFF;
                        objVec[objCount].g = (myPenColor.rgb() >> 8) & 0xFF;
                        objVec[objCount].b = (myPenColor.rgb()) & 0xFF;
                        objVec[objCount].x = (lastPoint.x()-250+(xRadius/2.0f))/50.0f;
                        objVec[objCount].y = (500-lastPoint.y()+(yRadius/2.0f))/50.0f;
                        objVec[objCount].z = objHeight;
                        objVec[objCount].xRad = (xRadius/50.0f);
                        objVec[objCount].yRad = (yRadius/50.0f);
                        objCount++; //Increment the count of total objects
                        modified = true;
                    }
                }
            }
        }


        scribbling = true;
    }
}

void FlyAR::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        scribbling = false;
    }
}

void FlyAR::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image, dirtyRect);
}

void FlyAR::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        update();
    }
    QWidget::resizeEvent(event);
}

void FlyAR::drawObj(const QPoint &endPoint)
{
    QPainter painter(&image);
        painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                            Qt::RoundJoin));

        if (objType == 1)
        {
            painter.drawEllipse(QRect(endPoint.x(), endPoint.y(), xRadius, yRadius));
        } else if (objType == 2)
        {
            painter.drawRect(QRect(endPoint.x(), endPoint.y(), xRadius, yRadius));
        }

        modified = true;

        int rad = (myPenWidth / 2) + 2;
        update(QRect(endPoint.x(), endPoint.y(), 25, 25).normalized()
                                         .adjusted(-rad, -rad, +rad, +rad));


}

void FlyAR::writeToFile()
{

    QFile file(outFileName);
    if (!file.isOpen())
    {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
    }
    QTextStream out(&file);

    for (vector<obj>::iterator it = objVec.begin(); it != objVec.end(); ++it)
    {
        out << QString::number(it->type) + ", ";
        out << QString::number(it->r) + ", ";
        out << QString::number(it->g) + ", ";
        out << QString::number(it->b) + ", ";
        out << QString::number(it->x) + ", ";
        out << QString::number(it->y) + ", ";
        out << QString::number(it->z) + ", ";
        out << QString::number(it->xRad) + ", ";
        out << QString::number(it->yRad) + "\n";
    }


    file.close();
}

void FlyAR::resizeImage(QImage *image, const QSize &newSize)
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}