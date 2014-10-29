#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include <QtGui>
#include <QObject>

#include "../appaco_lib/approximate_autocomplete.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTimer;
class QTreeWidget;
QT_END_NAMESPACE



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void popup_liste(QLineEdit *parent = 0);

    bool eventFilter(QObject *obj, QEvent *ev);
    void showCompletion(const QStringList &choices);

    inline char* get_queryWord();



public slots:
    void doneCompletion();
    void preventSuggest();
    void autoSuggest();

    void openFile();


private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_checkBox_clicked(bool checked);

    void on_radioButton_clicked(bool checked);

    void on_radioButton_2_clicked(bool checked);

    void on_radioButton_searchExact_clicked(bool checked);

    void on_radioButton_search_1error_clicked(bool checked);

private:
    Ui::MainWindow *ui;

    QTreeWidget *popup;

    QTimer *timer;

    Options *my_option;

    bool isDicConstructed;

    QString nameFileDictionary;


    int intervalSearch; // l'intervale entre chaque requeste
    bool isLenghtPrefixeByFunction; // quelle est la methode utiliser pour calculer le prefixe commun entre le queryword actuelle et le precedent,  (on a 2 methode by function "the best i Think", et by evenet)


    /// :::::::::::::::::: variable pour calculer le bon prefixe entre le mot precedent et le mot actuelle
    /// :::::::::::::::::: avec la methode des evente
    int lenght_PreviousWord; // size of the previeus word
    int charAdded; // char added to a previeus word
    bool isWordEditedInMiddle; // si le mot est editer au milieu

};

#endif // MAINWINDOW_H
