#include <string>
#include <vector>

namespace db {
    struct employee {
        int id = 0;
        int age = 0;
        int group_id = 0;
        double salary;
        std::string name = "Karl";
    };

    struct company_group {
        int id;
        std::string name = "Manager";
    };

    struct employee_list {
        std::string name;
        float state_of_drunk;
        std::vector<employee> employees;
    };
}