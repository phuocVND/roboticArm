#pragma once

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

struct ParameterAngle {
    short angle1;
    short angle2;
    short angle3;
    short angle4;
    short angle5;
    short angle6;
};

class Parameter : public QObject {
    Q_OBJECT
public:
    Parameter();
    ~Parameter();
    Q_INVOKABLE void set_parameter(const QList<double> &values);
    ParameterAngle* get_parameter();
    bool checkChangedPara();
    bool isChanged;
signals:
    void valueChanged();

private:

    void set_parameter_save();
    ParameterAngle *parameterAngle;
    ParameterAngle *parameterSaved;
};
