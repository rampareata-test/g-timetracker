import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import TimeLog 1.0
import "Texts.js" as Texts

ColumnLayout {
    property alias category: categoryModel.category

    spacing: 10

    TimeLogCategoryDepthModel {
        id: categoryModel

        timeTracker: TimeTracker
    }

    Repeater {
        model: categoryModel

        ItemPositioner {
            spacing: 10

            Label {
                Layout.fillWidth: true
                text: Texts.labelText("%1Category".arg(new Array(index+1).join("Sub")))
            }

            ComboBoxControl {
                id: combobox

                property int subcategory: subcategoryIndex === undefined ? -1 : subcategoryIndex

                currentIndex: subcategory
                model: subcategories ? [ "" ].concat(subcategories) : []

                onCurrentIndexChanged: {
                    if (subcategory != currentIndex) {
                        subcategoryIndex = currentIndex
                    }
                }

                onSubcategoryChanged: {
                    if (currentIndex != subcategory) {
                        currentIndex = subcategory
                    }
                }
            }
        }
    }
}
