#include "b15f.h"

B15F *B15F::instance = nullptr;
errorhandler_t B15F::errorhandler = nullptr;


/*************************************
 * Grundfunktionen des B15F Treibers *
 *************************************/

B15F &B15F::getInstance(void)
{
    if (!instance)
        instance = new B15F();

    return *instance;
}

void B15F::reconnect()
{
    uint8_t tries = RECONNECT_TRIES;
    while (tries--)
    {
        delay_ms(RECONNECT_TIMEOUT);
        discard();

        if (testConnection())
            return;
    }

    abort("Verbindung kann nicht repariert werden");
}

void B15F::discard(void)
{
    try
    {
        uint8_t rq[] =
        {
            RQ_DISCARD
        };

        usart.clearOutputBuffer();
        for (uint8_t i = 0; i < 16; i++)
        {
            usart.transmit(&rq[0], 0, sizeof(rq)); // sende discard Befehl (verwerfe input)
            delay_ms(4);
        }
        usart.clearInputBuffer();
    }
    catch (std::exception &ex)
    {
        abort(ex);
    }
}

bool B15F::testConnection()
{
    // erzeuge zufälliges Byte
    srand(time(NULL));
    uint8_t dummy = rand() % 256;

    uint8_t rq[] =
    {
        RQ_TEST,
        dummy
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw[2];
    usart.receive(&aw[0], 0, sizeof(aw));

    return aw[0] == MSG_OK && aw[1] == dummy;
}

bool B15F::testIntConv()
{
    srand(time(NULL));
    uint16_t dummy = rand() % (0xFFFF / 3);

    uint8_t rq[] =
    {
        RQ_INT_TEST,
        static_cast<uint8_t >(dummy & 0xFF),
        static_cast<uint8_t >(dummy >> 8)
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint16_t aw;
    usart.receive(reinterpret_cast<uint8_t *>(&aw), 0, sizeof(aw));

    return aw == dummy * 3;
}


std::vector<std::string> B15F::getBoardInfo(void)
{
    std::vector<std::string> info;

    uint8_t rq[] =
    {
        RQ_INFO
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t n;
    usart.receive(&n, 0, sizeof(n));
    while (n--)
    {
        uint8_t len;
        usart.receive(&len, 0, sizeof(len));

        char str[len + 1];
        str[len] = '\0';
        usart.receive(reinterpret_cast<uint8_t *>(&str[0]), 0, len);

        info.push_back(std::string(str));
    }

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    if (aw != MSG_OK)
        abort("Board Info fehlerhalft: code " + std::to_string((int) aw));

    return info;
}

void B15F::delay_ms(uint16_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void B15F::delay_us(uint16_t us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void B15F::reverse(uint8_t& b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
}

// https://stackoverflow.com/a/478960
std::string B15F::exec(std::string cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

void B15F::abort(std::string msg)
{
    DriverException ex(msg);
    abort(ex);
}

void B15F::abort(std::exception &ex)
{
    if (errorhandler)
        errorhandler(ex);
    else
    {
        std::cerr << "NOTICE: B15F::errorhandler not set" << std::endl;
        std::cout << ex.what() << std::endl;
        throw DriverException(ex.what());
    }
}

void B15F::setAbortHandler(errorhandler_t func)
{
    errorhandler = func;
}

/*************************************/



/*************************
 * Steuerbefehle für B15 *
 *************************/

bool B15F::activateSelfTestMode()
{
    uint8_t rq[] =
    {
        RQ_SELF_TEST
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

bool B15F::digitalWrite0(uint8_t port)
{
    uint8_t rq[] =
    {
        RQ_DIGITAL_WRITE_0,
        port
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

bool B15F::digitalWrite1(uint8_t port)
{
    uint8_t rq[] =
    {
        RQ_DIGITAL_WRITE_1,
        port
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

uint8_t B15F::digitalRead0()
{
    usart.clearInputBuffer();
    uint8_t rq[] =
    {
        RQ_DIGITAL_READ_0
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw;
}

uint8_t B15F::digitalRead1()
{
    usart.clearInputBuffer();
    uint8_t rq[] =
    {
        RQ_DIGITAL_READ_1
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw;
}

uint8_t B15F::readDipSwitch()
{
    usart.clearInputBuffer();
    uint8_t rq[] =
    {
        RQ_READ_DIP_SWITCH
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));

    reverse(aw); // DIP Schalter muss invertiert werden!

    return aw;
}

bool B15F::analogWrite0(uint16_t value)
{
    uint8_t rq[] =
    {
        RQ_ANALOG_WRITE_0,
        static_cast<uint8_t >(value & 0xFF),
        static_cast<uint8_t >(value >> 8)
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

bool B15F::analogWrite1(uint16_t value)
{
    uint8_t rq[] =
    {
        RQ_ANALOG_WRITE_1,
        static_cast<uint8_t >(value & 0xFF),
        static_cast<uint8_t >(value >> 8)
    };
    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

uint16_t B15F::analogRead(uint8_t channel)
{
    usart.clearInputBuffer();
    if (channel > 7)
        abort("Bad ADC channel: " + std::to_string(channel));

    uint8_t rq[] =
    {
        RQ_ANALOG_READ,
        channel
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint16_t aw;
    usart.receive(reinterpret_cast<uint8_t *>(&aw), 0, sizeof(aw));

    if (aw > 1023)
        abort("Bad ADC data detected (1)");
    return aw;
}

void
B15F::analogSequence(uint8_t channel_a, uint16_t *buffer_a, uint32_t offset_a, uint8_t channel_b, uint16_t *buffer_b,
                     uint32_t offset_b, uint16_t start, int16_t delta, uint16_t count)
{
    // prepare pointers
    buffer_a += offset_a;
    buffer_b += offset_b;


    usart.clearInputBuffer();
    uint8_t rq[] =
    {
        RQ_ADC_DAC_STROKE,
        channel_a,
        channel_b,
        static_cast<uint8_t >(start & 0xFF),
        static_cast<uint8_t >(start >> 8),
        static_cast<uint8_t >(delta & 0xFF),
        static_cast<uint8_t >(delta >> 8),
        static_cast<uint8_t >(count & 0xFF),
        static_cast<uint8_t >(count >> 8)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    for (uint16_t i = 0; i < count; i++)
    {
        if (buffer_a)
        {
            usart.receive(reinterpret_cast<uint8_t *>(&buffer_a[i]), 0, 2);

            if (buffer_a[i] > 1023) // check for broken usart connection
                abort("Bad ADC data detected (2)");
        }
        else
        {
            usart.drop(2);
        }

        if (buffer_b)
        {
            usart.receive(reinterpret_cast<uint8_t *>(&buffer_b[i]), 0, 2);

            if (buffer_b[i] > 1023) // check for broken usart connection
                abort("Bad ADC data detected (3)");
        }
        else
        {
            usart.drop(2);
        }
    }

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    if(aw != MSG_OK)
        abort("Sequenz unterbrochen");
}

uint8_t B15F::pwmSetFrequency(uint32_t freq)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_PWM_SET_FREQ,
        static_cast<uint8_t>((freq >> 0) & 0xFF),
        static_cast<uint8_t>((freq >> 8) & 0xFF),
        static_cast<uint8_t>((freq >> 16) & 0xFF),
        static_cast<uint8_t>((freq >> 24) & 0xFF)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw;
}

bool B15F::pwmSetValue(uint8_t value)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_PWM_SET_VALUE,
        value
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == MSG_OK;
}

bool B15F::setMem8(volatile uint8_t* adr, uint8_t val)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_SET_MEM_8,
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) & 0xFF),
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) >> 8),
        val
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw == val;
}

uint8_t B15F::getMem8(volatile uint8_t* adr)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_GET_MEM_8,
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) & 0xFF),
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) >> 8)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    return aw;
}

bool B15F::setMem16(volatile uint16_t* adr, uint16_t val)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_SET_MEM_16,
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) & 0xFF),
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) >> 8),
        static_cast<uint8_t >(val & 0xFF),
        static_cast<uint8_t >(val >> 8)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint16_t aw;
    usart.receive(reinterpret_cast<uint8_t *>(&aw), 0, sizeof(aw));
    return aw == val;
}

uint16_t B15F::getMem16(volatile uint16_t* adr)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_GET_MEM_16,
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) & 0xFF),
        static_cast<uint8_t >(reinterpret_cast<size_t>(adr) >> 8)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint16_t aw;
    usart.receive(reinterpret_cast<uint8_t *>(&aw), 0, sizeof(aw));
    return aw;
}

bool B15F::setRegister(volatile uint8_t* adr, uint8_t val)
{
    return setMem8(adr, val);
}

uint8_t B15F::getRegister(volatile uint8_t* adr)
{
    return getMem8(adr);
}

uint16_t* B15F::getInterruptCounterOffset()
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_COUNTER_OFFSET
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint16_t aw;
    usart.receive(reinterpret_cast<uint8_t *>(&aw), 0, sizeof(aw));
    return reinterpret_cast<uint16_t*>(aw);
}

void B15F::setServoEnabled(void)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_SERVO_ENABLE
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    assertCode(aw, MSG_OK);
}

void B15F::setServoDisabled(void)
{
    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_SERVO_DISABLE
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    assertCode(aw, MSG_OK);
}

void B15F::setServoPosition(uint16_t pos)
{
    if(pos > 19000)
        throw DriverException("Impulslänge ist zu lang: " + std::to_string(pos));

    usart.clearInputBuffer();

    uint8_t rq[] =
    {
        RQ_SERVO_SET_POS,
        static_cast<uint8_t >(pos & 0xFF),
        static_cast<uint8_t >(pos >> 8)
    };

    usart.transmit(&rq[0], 0, sizeof(rq));

    uint8_t aw;
    usart.receive(&aw, 0, sizeof(aw));
    assertCode(aw, MSG_OK);
}

/*************************/


/**********************
 * Private Funktionen *
 **********************/

B15F::B15F()
{
    init();
}


void B15F::init()
{

    std::string device = exec("bash -c 'ls /dev/ttyUSB*'");
    while (device.find(' ') != std::string::npos || device.find('\n') != std::string::npos ||
            device.find('\t') != std::string::npos)
        device.pop_back();

    if (device.length() == 0)
        abort("Adapter nicht gefunden");

    std::cout << PRE << "Verwende Adapter: " << device << std::endl;


    std::cout << PRE << "Stelle Verbindung mit Adapter her... " << std::flush;
    usart.setBaudrate(BAUDRATE);
    usart.openDevice(device);
    std::cout << "OK" << std::endl;


    std::cout << PRE << "Teste Verbindung... " << std::flush;
    uint8_t tries = 3;
    while (tries--)
    {
        // verwerfe Daten, die µC noch hat
        //discard();

        if (!testConnection())
            continue;

        if (!testIntConv())
            continue;

        break;
    }
    if (tries == 0)
        abort("Verbindungstest fehlgeschlagen. Neueste Version im Einsatz?");
    std::cout << "OK" << std::endl;


    // Gib board info aus
    std::vector<std::string> info = getBoardInfo();
    std::cout << PRE << "AVR Firmware Version: " << info[0] << " um " << info[1] << " Uhr (" << info[2] << ")"
              << std::endl;
}

void B15F::assertCode(uint8_t& code, uint8_t expectation) const
{
    if(code != expectation)
        throw DriverException("Ungültige Antwort erhalten: " + std::to_string((int) code) + " (erwartet: " + std::to_string((int) expectation) + ")");
}
