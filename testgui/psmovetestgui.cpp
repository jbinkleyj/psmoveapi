#include "psmovetestgui.h"
#include "ui_psmovetestgui.h"

#include "psmovesensorscene.h"

#include "devicechooserdialog.h"

PSMoveTestGUI::PSMoveTestGUI(QWidget *parent) :
    QMainWindow(parent),
    psm(NULL),
    colorLEDs(Qt::black),
    _chosenIndex(-1),
    _red(NULL),
    _green(NULL),
    ui(new Ui::PSMoveTestGUI)
{
    ui->setupUi(this);

    on_checkboxEnable_toggled(false);
    setColor(Qt::red);
}

PSMoveTestGUI::~PSMoveTestGUI()
{
    delete ui;
    delete psm;
}

void PSMoveTestGUI::on_checkboxEnable_toggled(bool checked)
{
    ui->groupLEDs->setEnabled(checked);
    ui->groupRumble->setEnabled(checked);
    ui->groupSensors->setEnabled(checked);
    ui->groupTrigger->setEnabled(checked);
    ui->groupBluetooth->setEnabled(checked);

    if (checked) {
        int index = 0;

        switch (PSMoveQt::count()) {
            case 0:
                QMessageBox::warning(this, "Cannot connect",
                    "No PS Move controller found. "
                    "Please connect one via USB or Bluetooth.");
                ui->checkboxEnable->toggle();
                return;
                break;
            case 1:
                index = 0;
                break;
            default:
                if (_chosenIndex >= 0) {
                    index = _chosenIndex;
                    _chosenIndex = -1;
                } else {
                    ui->checkboxEnable->toggle();
                    _red = new PSMoveQt(0);
                    _red->setColor(Qt::red);
                    _green = new PSMoveQt(1);
                    _green->setColor(Qt::green);
                    _chosenIndex = -1;
                    (new DeviceChooserDialog(this))->show();
                    return;
                }
                break;
        }

        psm = new PSMoveQt(index);

        switch (psm->connectionType()) {
            case PSMoveQt::Bluetooth:
                ui->checkboxEnable->setText("Enable PS Move controller (connected via Bluetooth)");
                break;
            case PSMoveQt::USB:
                ui->checkboxEnable->setText("Enable PS Move controller (connected via USB)");
                break;
            default:
                break;
        }

        connect(psm, SIGNAL(triggerChanged()),
                this, SLOT(setTrigger()));

        connect(psm, SIGNAL(buttonPressed(int)),
                this, SLOT(onButtonPressed(int)));

        connect(psm, SIGNAL(buttonReleased(int)),
                this, SLOT(onButtonReleased(int)));

        //ui->viewSensors->setScene(new PSMoveSensorScene(this, psm));

        psm->setRumble(ui->sliderRumble->value());
        psm->setColor(colorLEDs);
        psm->setEnabled(true);
    } else {
        if (psm != NULL) {
            psm->setRumble(0);
            psm->setColor(Qt::black);
            psm->setEnabled(false);
            delete psm;
        }
        psm = NULL;
        ui->checkboxEnable->setText("Enable PS Move controller");
    }
}

void PSMoveTestGUI::setColor(QColor color)
{
    colorLEDs = color;
    ui->labelLEDs->setText("<div style='background-color: "+colorLEDs.name()+
                           "; color: "+colorLEDs.name()+";'>LEDs</div>");

    if (psm != NULL && psm->enabled()) {
        psm->setColor(color);
    }
}

void PSMoveTestGUI::on_buttonLEDs_clicked()
{
    QColor result = QColorDialog::getColor(colorLEDs, this);

    if (result.isValid()) {
        setColor(result);
    }
}

void PSMoveTestGUI::on_buttonQuit_clicked()
{
    QApplication::quit();
}

void PSMoveTestGUI::setTrigger()
{
    if (psm != NULL) {
        ui->progressbarTrigger->setValue(psm->trigger());
    }
}

void PSMoveTestGUI::on_sliderRumble_sliderMoved(int position)
{
    if (psm != NULL && psm->enabled()) {
        psm->setRumble(position);
    }
}

void PSMoveTestGUI::onButtonPressed(int button)
{
    setButton(button, true);
}

void PSMoveTestGUI::onButtonReleased(int button)
{
    setButton(button, false);
}

void PSMoveTestGUI::setButton(int button, bool pressed)
{
    switch ((PSMoveQt::ButtonType)button) {
        case PSMoveQt::Start:
            ui->checkboxStart->setChecked(pressed);
            break;
        case PSMoveQt::Select:
            ui->checkboxSelect->setChecked(pressed);
            break;
        case PSMoveQt::T:
            ui->checkboxTrigger->setChecked(pressed);
            break;
        case PSMoveQt::Move:
            ui->checkboxMove->setChecked(pressed);
            break;
        case PSMoveQt::Square:
            ui->checkboxSquare->setChecked(pressed);
            break;
        case PSMoveQt::Triangle:
            ui->checkboxTriangle->setChecked(pressed);
            break;
        case PSMoveQt::Cross:
            ui->checkboxCross->setChecked(pressed);
            break;
        case PSMoveQt::Circle:
            ui->checkboxCircle->setChecked(pressed);
            break;
        case PSMoveQt::PS:
            ui->checkboxPS->setChecked(pressed);
            break;
        default:
            break;
    }
}

void PSMoveTestGUI::on_buttonBluetoothSet_clicked()
{
    if (psm != NULL && psm->connectionType() == PSMoveQt::Bluetooth) {
        QMessageBox::information(this, "Already paired",
            "The controller is already connected. No need for pairing.");
    } else if (psm != NULL && psm->connectionType() != PSMoveQt::USB) {
        QMessageBox::warning(this, "Could not pair",
            "Please connect your controller via USB.");
    } else if (psm != NULL && psm->pair()) {
        QMessageBox::information(this, "Pairing successful",
            "Disconnect the USB cable, "
            "then press the PS button to connect via Bluetooth.");
        ui->checkboxEnable->toggle();
    } else {
        QMessageBox::warning(this, "Could not pair",
            "Please make sure your PS Move is connected via USB.");
    }
}

void PSMoveTestGUI::reconnectByIndex(int index)
{
    _chosenIndex = index;

    if (_red != NULL) {
        delete _red;
        _red = NULL;
    }

    if (_green != NULL) {
        delete _green;
        _green = NULL;
    }

    if (index != -1) {
        ui->checkboxEnable->toggle();
    }
}