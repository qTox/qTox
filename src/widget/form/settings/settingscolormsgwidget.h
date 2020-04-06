#ifndef SETTINGSCOLORMSGWIDGET_H
#define SETTINGSCOLORMSGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>

class SettingsColorMsgWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsColorMsgWidget(QWidget* parent = nullptr);

    void setColor(const QString& c);
    void setCheckedBold(bool state);
    void setCheckedItalic(bool state);

private:
    QPushButton* chooseColorButton;
    QCheckBox* boldCheckBox;
    QCheckBox* italicCheckBox;

    QColor color;

private slots:
    void chooseColor();

signals:
    void selectedColor(const QString& color);
    void checkBold(bool state);
    void checkItalic(bool state);
};

#endif // SETTINGSCOLORMSGWIDGET_H
