#include "mainwindow.h"
#include "ui_mainwindow.h"

#define INTERVAL_SEARSH 200 // interval in millisecond (la valeur par defaut)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEdit->installEventFilter(this);

    popup_liste(this->ui->lineEdit);

    connect(this->ui->lineEdit, SIGNAL(returnPressed()),this, SLOT(preventSuggest()));

    connect(ui->actionOpen_Dict, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));



    my_option = new_options();
    isDicConstructed = false;

    ui->comboBox_2->setCurrentIndex(my_option->typeSearch);
    ui->comboBox->setCurrentIndex(2); // 200 Ms

    ui->checkBox->setChecked(true); // affichage avec top_K

    isLenghtPrefixeByFunction=true;
    ui->radioButton->setChecked(isLenghtPrefixeByFunction);

    ui->radioButton_searchExact->setChecked(my_option->isExactSearchMethod);
    ui->radioButton_search_1error->setChecked(!my_option->isExactSearchMethod);

    return;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::popup_liste(QLineEdit *parent)
{

    popup = new QTreeWidget;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);
    popup->setMouseTracking(true);

    popup->setColumnCount(1);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();



    popup->installEventFilter(this);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(doneCompletion()));

    timer = new QTimer();
    timer->setSingleShot(true);
    intervalSearch = INTERVAL_SEARSH; // la valeur par defaut
    timer->setInterval(intervalSearch); // cette valeur sera changer par l'utilisateur (l'event from comboBox)
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(this->ui->lineEdit, SIGNAL(textEdited(QString)), timer, SLOT(start()));

}

// ibra :
// we use "Qt::Key_Delete" to konw if we deleted some char frome pattern
// and we use one boolean variable to know if we add some charactère after deletion
//
// 1) comme sa on peut savoire si on est entraine d'ajoute des charactère pour un prefixe
// ou
// 2) supprimer des charactère depuit le pattern
// ou
// 3) supprimer ensuit ajouter d'autres char
// donc:
// 1) j'ai déja fait sa (la fonction est prète)
// 3) elle resmble a (1), donc il suffi de recupirer le prefixe où on arriter au moment de
//    la suppresion,  ensuit on commence la recherche pour le prefixe depuit se point là
//    (3') aussi: il se peut que on a sauvgarder un mot, ensuit on supprime quellque char, ensuit on ajout d'autre
//                donc la solution est recupirer le point (le node) avec la supprission ensuit on fait la recherche pour le suffixe ajouter.
// 2) j'ai commancer a programer la fonction, mais elle n'est pas complet, elle ne marche pas encoure
//
// je pense que on juste ces trois cas la
//
// mais mais mais   est ce que sa vaut la peine de les faire en terme de temps d'execution
// pour le 1 et 3 je pense que yes;
// pour le 2 je ne sais pas.
//
// mais je pense que le plus important est le top k,  je doit commencer avec lui :)



///  ibra : this eventFilter is used when we calculate the common prefixe by the evente key pressed
//// remarque : popup && lineEdit eventFilet :
bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{

    if(obj!=popup && obj!=ui->lineEdit ) /// probleme here;
        return false; // si l'event vien d'un autres source, donc on doit le laisser, on doit pas le blocker

    /// qDebug()<<" obj = "<<obj;

    if (obj == popup)
    {
        if (ev->type() == QEvent::MouseButtonPress)
        {
            popup->hide(); // cacher le menu popup
            this->ui->lineEdit->setFocus(); // et redonner le focus a lineEdit
            return true; // et blocker l'event pour le menu popup
        }


        if (ev->type() == QEvent::KeyPress)
        {
            int key = static_cast<QKeyEvent*>(ev)->key();
            switch (key)
            {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    doneCompletion();
                    return true; // blocker l'event pour le menu popup

                case Qt::Key_Escape:
                    this->ui->lineEdit->setFocus();
                    popup->hide();
                    return true; // blocker l'event pour le menu popup

                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Home:
                case Qt::Key_End:
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                    return false; //  ne pas blocker ces evenement, car on a besoine de ce deplacer dans le menu popup pour choisir un mot

                default:
                    this->ui->lineEdit->setFocus(); // donner le Focus a lineEdit
                    this->ui->lineEdit->event(ev);  // rederiger l'evenement vers lineEdit
                    popup->hide();                  // cacher le menu popup
                    return false;                   // ne pas bloker cette evenement de popup
                    /// ibra : normalement on block l'evenement pour popup car on a le rederger vers lineEdit,
                    /// mais car le menu popup est cacher donc sa ne fait rien
            }
        }


    }
    else
    {
        if (obj == ui->lineEdit) // ce if est inutil, car normalement forçement on est dans le cas lineEdit
        {
            if (ev->type() == QEvent::KeyPress)
            {
                bool consumed = false;
                int key = static_cast<QKeyEvent*>(ev)->key();
                switch (key)
                {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_Escape:
                    // return true; // blocker ces event
                    consumed = true;
                    break;

                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Home:
                case Qt::Key_End:
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                    // return false; // ne pas blocker ces event, pour laisser le deplacement dans lineEdit
                    break;

                case Qt::Key_Delete:
                    if(!isWordEditedInMiddle)// le mot n'est pas editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition()) // l'editon est a la fin
                        {
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            lenght_PreviousWord = ui->lineEdit->cursorPosition();
                            isWordEditedInMiddle = true;
                        }
                    }
                    else /// isWordEditedInMiddle: le mot est editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition())
                        {
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            if(lenght_PreviousWord > ui->lineEdit->cursorPosition())
                            {
                                lenght_PreviousWord = ui->lineEdit->cursorPosition();
                                // isWordEditedInMiddle = true; // we didn't need this instruction here cause this variable wase already changed
                            }
                        }
                    }
                    /// return false; // ne pas blcker cette event,
                    break;

                case Qt::Key_Backspace:
                    if(!isWordEditedInMiddle)// le mot n'est pas editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition()) // l'editon est a la fin
                        {
                            if(charAdded==0 )
                            {
                                if(lenght_PreviousWord!=0) lenght_PreviousWord --;
                            }
                            else charAdded --;
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            if(lenght_PreviousWord != 0) // cause if lenght_word == 0 && cursorPosition()==0, we fall in nigative number
                            {
                                lenght_PreviousWord = ui->lineEdit->cursorPosition()-1;
                                isWordEditedInMiddle = true;
                            }
                        }
                    }
                    else /// isWordEditedInMiddle: le mot est editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition())
                        {
                            if(lenght_PreviousWord == ui->lineEdit->cursorPosition())
                            {
                                if(charAdded==0)
                                {
                                    if(lenght_PreviousWord != 0)
                                        lenght_PreviousWord --;
                                }
                                else charAdded --;
                            }
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            if(lenght_PreviousWord > ui->lineEdit->cursorPosition())
                            {
                                lenght_PreviousWord = ui->lineEdit->cursorPosition()-1;
                                /// isWordEditedInMiddle = true;
                            }
                        }
                    }
                    /// return false; // ne pas blcker cette event,
                    break;

                default:
                    if(!isWordEditedInMiddle)// le mot n'est pas editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition()) // l'editon est a la fin
                        {
                            charAdded++;
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            lenght_PreviousWord = ui->lineEdit->cursorPosition();
                            isWordEditedInMiddle = true;
                        }
                    }
                    else /// isWordEditedInMiddle: le mot est editer au milieu
                    {
                        if(ui->lineEdit->text().size()==ui->lineEdit->cursorPosition())
                        {
                            charAdded++;
                        }
                        else // here we have ( text.size > cursorPosition )
                        {
                            if(lenght_PreviousWord > ui->lineEdit->cursorPosition())
                            {
                                lenght_PreviousWord = ui->lineEdit->cursorPosition();
                                /// isWordEditedInMiddle = true;
                            }
                        }
                    }
                    /// return false; // ne pas blcker cette event,
                    break;
                }

                return consumed;
            }
        }
    }

    return false; // false : pour ne pas bloker l'evente
}



void MainWindow::showCompletion(const QStringList &choices)
{

    if (choices.isEmpty())
        return;

    popup->setUpdatesEnabled(false);
    popup->clear();


    for (int i = 0; i < choices.count(); ++i)
    {
        QTreeWidgetItem * item;
        item = new QTreeWidgetItem(popup);
        item->setText(0, choices[i]);
    }

    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int h = popup->sizeHintForRow(0) * qMin(7, choices.count()) + 3;
    popup->resize(popup->width(), h);

    popup->move(this->ui->lineEdit->mapToGlobal(QPoint(0, this->ui->lineEdit->height())));
    popup->setFocus();
    popup->show();

    return;
}



void MainWindow::doneCompletion()
{
    timer->stop();
    popup->hide();
    this->ui->lineEdit->setFocus();
    QTreeWidgetItem *item = popup->currentItem();
    if (item)
    {
        this->ui->lineEdit->setText(item->text(0));
        QMetaObject::invokeMethod(this->ui->lineEdit, "returnPressed");
    }

    /*
    // faire la modification pour le scoure de string choiser
    if(search_info->typePrintResults == PRINT_WORD_LIST_TOP_K)
    {
        int numStringChoised =  popup->currentIndex().row();
        qDebug()<< " topK_modified row = "<<popup->currentIndex().row() ;

        topK_modified_AllWayNodes_bySearching(numStringChoised,trie_info,listStringInfo);
    }

    */

    return;
}



void MainWindow::autoSuggest()
{
    // si on pas crier le dictionnaire, on doit le crier avant de faire le traitement.
    if(!isDicConstructed)
    {
        QMessageBox::warning(this, tr("MDI"),
                         tr(" dictionnaire not constructed  \n File--> open ."));
        ui->lineEdit->clear();

        return;
    }



    char *queryWord = get_queryWord();

    qDebug()<< " queryWord = " << queryWord ;


    ListString* choices=get_results(queryWord);

    QStringList choices_QString;
    int i;

    for(i=0; i<choices->size; i++)
    {
        choices_QString.append(QString::fromUtf8(choices->buffer[i]));
    }

    showCompletion(choices_QString);

    return;
}



void MainWindow::preventSuggest()
{
    timer->stop();
}



void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(index==0)
    {
        intervalSearch = 50;
    }
    else
    {
        intervalSearch = index * 100;
    }

    timer->setInterval(intervalSearch);

    qDebug()<< " currentIndexChanged = " << index << " intervalSearch = "<<intervalSearch;

    return;
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    my_option->typeSearch = index;

    return;
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    qDebug()<< " on_checkBox_clicked = " <<  checked;

    if(checked)
    {
        my_option->typePrintResults = PRINT_WORD_LIST_TOP_K;
    }
    else
    {
        my_option->typePrintResults = PRINT_WORD_LIST;
    }

    return;
}

void MainWindow::on_radioButton_clicked(bool checked)
{
    // isLenghtPrefixeByFunction = checked;
    isLenghtPrefixeByFunction = true;
    qDebug()<< " on_radioButton_clicked : isLenghtPrefixeByFunction  = "<< isLenghtPrefixeByFunction ;

    return;
}

void MainWindow::on_radioButton_2_clicked(bool checked)
{
    // isLenghtPrefixeByFunction = ui->radioButton->isChecked();
    isLenghtPrefixeByFunction = false;
    qDebug()<< " on_radioButton_2_clicked : isLenghtPrefixeByFunction  = "<< isLenghtPrefixeByFunction ;

    return;
}


void MainWindow::openFile()
{
    // recuperer le nom de fichier a ouvrir
    nameFileDictionary = QFileDialog::getOpenFileName(this, tr("Open File..."),
                                                    QString(), tr("Text files (*.txt);;All Files (*)"));

    qDebug()<< " nameFileDictionary = "<<nameFileDictionary;

    /// search_info->dic_NameFile = (char*)malloc((nameFileDictionary.toAscii().size())*sizeof(char));

    /// strcpy(search_info->dic_NameFile,nameFileDictionary.toAscii().data());

    /// qDebug()<< " search_info->dic_NameFile = "<<search_info->dic_NameFile;

// --------------------------------------------
    // 1st phase : initialisation

    // Build the tree and retrieve information necessary for the search
    char *source = nameFileDictionary.toAscii().data();

    initialize_AutoComplete(source,my_option);

 // ----------------------------------------------


    isDicConstructed = true;

   qDebug("\n\n ::::::::::::::::::::::::::::::::: MAIN :");

    return;
}

void MainWindow::on_radioButton_searchExact_clicked(bool checked)
{
    my_option->isExactSearchMethod=true;

    qDebug()<<" search_info->isExactSearchMethode = "<<my_option->isExactSearchMethod;

    return;
}

void MainWindow::on_radioButton_search_1error_clicked(bool checked)
{
    my_option->isExactSearchMethod=false;
    qDebug()<<" search_info->isExactSearchMethode = "<<my_option->isExactSearchMethod;

    return;
}


// recupirer le mot requete depuit un fichier UTF-8, car en mode console en peut pas faire cela
inline char* MainWindow::get_queryWord()
{
    char *queryWord =  (char *) malloc((ui->lineEdit->text().toUtf8().size()+1)*sizeof(char)); // +1 : '\0'
    strcpy(queryWord,ui->lineEdit->text().toUtf8().data());

    return queryWord;
}
