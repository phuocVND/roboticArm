#include "parameter.h"
#include <QtCore/qlist.h>
#include <QDebug>
#include <iostream>

Parameter::Parameter()
{
    parameterAngle = new ParameterAngle();
    parameterSaved = new ParameterAngle();

    this->parameterAngle->angle1 = 0;
    this->parameterAngle->angle2 = 0;
    this->parameterAngle->angle3 = 0;
    this->parameterAngle->angle4 = 0;
    this->parameterAngle->angle5 = 0;
    this->parameterAngle->angle6 = 0;
    this->isChanged = false;
}

Parameter::~Parameter() {
    delete parameterAngle;
    delete parameterSaved;
}

void Parameter::set_parameter_save()
{
    this->parameterSaved->angle1 = this->parameterAngle->angle1;
    this->parameterSaved->angle2 = this->parameterAngle->angle2;
    this->parameterSaved->angle3 = this->parameterAngle->angle3;
    this->parameterSaved->angle4 = this->parameterAngle->angle4;
    this->parameterSaved->angle5 = this->parameterAngle->angle5;
    this->parameterSaved->angle6 = this->parameterAngle->angle6;
}

bool Parameter::checkChangedPara()
{
    this->isChanged =
        this->parameterAngle->angle1 != this->parameterSaved->angle1 ||
        this->parameterAngle->angle2 != this->parameterSaved->angle2 ||
        this->parameterAngle->angle3 != this->parameterSaved->angle3 ||
        this->parameterAngle->angle4 != this->parameterSaved->angle4 ||
        this->parameterAngle->angle5 != this->parameterSaved->angle5 ||
        this->parameterAngle->angle6 != this->parameterSaved->angle6;

    return this->isChanged;
}

void Parameter::set_parameter(const QList<double> &values)
{
    if (values.size() < 6) {
        qWarning() << "Error: Input list must have at least 6 values";
        return;
    }

    // qWarning() << values <<  " ";
    this->parameterAngle->angle1 = values[0];
    this->parameterAngle->angle2 = values[1];
    this->parameterAngle->angle3 = values[2];
    this->parameterAngle->angle4 = values[3];
    this->parameterAngle->angle5 = values[4];
    this->parameterAngle->angle6 = values[5];

    if (checkChangedPara()) {
        set_parameter_save();
        emit valueChanged();
    }
}

ParameterAngle* Parameter::get_parameter()
{
    this->isChanged = false;
    return this->parameterSaved;
}
