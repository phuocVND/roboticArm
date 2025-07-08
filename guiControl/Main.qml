import QtQuick
import QtQuick.Controls
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    ListModel {
        id: sliderModel
        ListElement { name: "1";min: 0; max: 180; val: 160 }
        ListElement { name: "2";min: 0; max: 180; val: 40 }
        ListElement { name: "3";min: 0; max: 180; val: 90 }
        ListElement { name: "4";min: 0; max: 180; val: 0 }
        ListElement { name: "5";min: 0; max: 180; val: 90 }
        ListElement { name: "6";min: 0; max: 70; val: 0 }

    }
    Column {
        spacing: 10

        Repeater {
            model: sliderModel

            Column {
                spacing: 4

                Text {
                    text: model.name
                    font.bold: true
                }

                Slider {
                    from: model.min
                    to: model.max
                    value: model.val
                    width: 200
                    onValueChanged: {

                        sliderModel.setProperty(index, "val", value);
                        var values = getSliderValues();
                        parameter.set_parameter(values);
                        // console.log("Parameters set:", values);
                    }
                }
            }
        }
    }

    // Connections {
    //     target: parameter
    // }

    function getSliderValues() {
            var values = [];
            for (var i = 0; i < sliderModel.count; i++) {
                values.push(sliderModel.get(i).val);
            }
            return values;
        }
}
