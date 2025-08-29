#include <iostream>
#include <cassert>

#include "../test/mocks/spi_mock.h"
#include "../test/mocks/spi_mock_p.h"
#include "../include/spi.h"
#include "../include/mram_mr25h40.h"
#include "../include/bureau_store.h"

int main(int argc, char* argv[]) {

    SpiMockP spi_p;
    SpiMock spi_s;
    Spi *spi_mock = nullptr;

    if(argc == 2 && std::string(argv[1]) == "P") {
        
        spi_mock = &spi_p;
    }else{
        
        spi_mock = &spi_s;
    }

    MR25H40 mram(*spi_mock);
    mram.power_up_delay();

    BureauStore store(mram);

    // --- Часть 1. Базовая запись/чтение Bureau ---
    Bureau w{
        .prog_qty = 123456u,
        .math_qty=42u,
        .head_qty=3u,
        .salary_sum=12345.75f
    };
    
    store.write(w);
    Bureau r = store.read();

    assert(r.prog_qty==w.prog_qty && r.math_qty==w.math_qty &&
           r.head_qty==w.head_qty && r.salary_sum==w.salary_sum);

    std::cout << "Bureau OK: " << r.prog_qty << " / " << r.salary_sum << std::endl;

    // --- Часть 2. Демонстрация защиты ---
    std::cout << "\n--- Demonstrating block protect ---" << std::endl;

    // Снимаем любую защиту (для чистоты эксперимента)
    mram.set_block_protect(MR25H40::Protect::None);

    // Запишем в верхнюю четверть памяти (адрес 0x60000)
    std::array<uint8_t, 4> test_data1{1,2,3,4};
    mram.write(0x60000, test_data1);

    // Прочитаем обратно
    std::array<uint8_t, 4> readback1{};
    mram.read(0x60000, readback1);
    std::cout << "Write OK before protection: " << int(readback1[0]) << "," << int(readback1[1]) << std::endl;

    // Включаем защиту верхней четверти
    mram.set_block_protect(MR25H40::Protect::UpperQuarter);

    // Пытаемся перезаписать то же место
    std::array<uint8_t, 4> test_data2{9,9,9,9};
    mram.write(0x60000, test_data2);

    // Читаем снова
    std::array<uint8_t, 4> readback2{};
    mram.read(0x60000, readback2);

    std::cout << "After protection attempt: " << int(readback2[0]) << "," << int(readback2[1]) << std::endl;

    if(readback2[0] == test_data1[0] && readback2[1] == test_data1[1]){

        std::cout << "✅ Protection works: data did NOT change" << std::endl;
    } else {

        std::cout << "⚠️ Data changed (protection not active in mock)" << std::endl;
    }

    return 0;
}
