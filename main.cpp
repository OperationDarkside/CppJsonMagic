#include <print>

#include "json_magic.cpp"
#include "db_classes.cpp"

int main()
{
    db::company_group g1{.id = 55, .name = "Specialists"};
    auto g1_as_string = json_magic::to_string(g1);
    std::println("\ng1: {}", g1_as_string);

    db::employee e1{.id = 47, .age = 48, .group_id = 0, .salary = 999999.99, .name = "Agent"};
    auto e1_as_string = json_magic::to_string(e1);
    std::println("\ne1: {}", e1_as_string);

    db::employee e1_copy = json_magic::from_string<db::employee>(e1_as_string);

    std::println("\ne1 deserialized from json\n{} {} {} {} {}", e1_copy.id, e1_copy.age, e1_copy.group_id, e1_copy.salary, e1_copy.name);

    db::employee e2{.id = 37, .age = 22, .group_id = 3, .salary = 3.50, .name = "Intern"};

    db::employee_list e_list{.name = "Office Party", .state_of_drunk = 2.3, .employees = {e1, e2}};
    auto e_list_as_string = json_magic::to_string(e_list);
    std::println("\ne_list: {}", e_list_as_string);

    db::employee_list e_list_copy = json_magic::from_string<db::employee_list>(e_list_as_string);
    std::println("\ne_list_copy: name: {} drunk%: {}\nemployee1 name: {}\nemployee1 name: {}", e_list_copy.name, e_list_copy.state_of_drunk, e_list_copy.employees[0].name, e_list_copy.employees[1].name);

    return 0;
}