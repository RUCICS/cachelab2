// clang-format off
#include "cachelab.h"

// demo 函数会演示部分你可以/不可以使用的语法
// 事实上，多数情况只要不在编译期报错的语法就是可以写的

/*
要编译并运行本文件，请执行以下命令:
> make demo
> ./demo
*/ 

#ifndef USE_EXPLICIT

reg example_return(ptr_reg& ptr);

void demo(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    /********** 基础用法 **********/
    reg a = A[0];             // 初始化一个寄存器，并读内存到寄存器
    reg b = 100;              // 初始化一个寄存器，并初始化一个常数，这不会导致内存访问，默认初始化为 0
    b = B[10];                // 读内存到寄存器
    B[10] = a;                // 将寄存器写入内存
    A[0] = 0;                 // 这会产生一条特殊的内存访问记录，其寄存器编号为 -1，代表不是来着于寄存器，而是立即数
                              // 其开销等于从寄存器到内存
    ptr_reg subC = C + 10;  // 初始化一个指针，指针也会占用一个寄存器，注意函数传入的参数也占用了寄存器
    reg var[2];               // 你可以申请寄存器数组
    var[a] = 0;               // 并用寄存器作为下标

    // int mem1;                 // 但你不能申请内存，或者内存数组
    // int mem2[2];              // 但你不能申请内存，或者内存数组

    reg c = *subC;                                          // 用 * 操作符读内存到寄存器
    reg d = A[a * b * (c + 5)];                             // 寄存器运算的结果可以作为下标使用
    [[maybe_unused]] ptr_reg subA = A + a * (b + 5) * c;  // 也可以用于指针运算

    std::cout << "Current: " << get_current_reg_count() << std::endl;
    reg* reg_array = new reg[2];                        // 你可以申请寄存器数组，请注意不要使用 C 风格的 malloc；除了 new，你还可以使用 C++ 风格的智能指针
    std::cout << "Current: " << get_current_reg_count() << std::endl;
    delete[] reg_array;                                 // 不要忘记释放寄存器数组
    std::cout << "Current: " << get_current_reg_count() << std::endl;
    /*****************************/

    /********** 特殊情况 **********/
    // 大多数 +-*/% 操作都是可以用的，除了下面这个情况
    // a--;                                // 我们删掉了 operator--(int) 和 operator++(int) 操作符
    //                                     // 因为这事实上需要临时保存原来的值以返回，意味着一个额外的寄存器占用
    //                                     // 如果你确实需要这个行为，你可以分成两步完成
    b = a;
    --b;
    /*****************************/


    /********** 禁用用法 **********/
    // 以下操作会在编译期报错

    // A[0] * B[0];      // 内存上的数据不能直接参与计算
    // error: no match for ‘operator*’ (operand types are ‘MemoryWarpper<int>’ and ‘MemoryWarpper<int>’)

    // A[0] * a;         // 内存上的数据不能直接参与计算
    // error: no match for ‘operator*’ (operand types are ‘MemoryWarpper<int>’ and ‘reg’ {aka ‘RegisterWarpper<int>’})
    // 1 + A[0];
    // error: no match for ‘operator+’ (operand types are ‘int’ and ‘MemoryWarpper<int>’)

    // A[0] = *(B + 1);  // 不能直接内存到内存赋值
    // error: use of deleted function ‘constexpr MemoryWarpper<int>& MemoryWarpper<int>::operator=(const MemoryWarpper<int>&)’
    /*****************************/


    /********** 高级用法 **********/
    reg old_reg = A[10];
    std::cout << old_reg.info() << std ::endl;  // $4(ACTIVE): 10
    // 在 DEBUG 的时候你可以查看寄存器信息，比如寄存器序号和当前状态

    std::cout << old_reg << std::endl;
    // 也可以直接输出寄存器的值

    std::cout << std::hex << A << std::dec << std::endl;
    std::cout << A.info() << std::endl;
    // 还可以输出指针的值，即指向的地址，注意默认总是 10 进制输出，所以你可以用 std::hex << A 输出 16 进制

    // 有时候你希望把一个寄存器绑定撤销，也就是之前的一个 reg 再也不用了，你不希望它占用一个寄存器，你可以用以下方法

    // 方法1，给寄存器一个显式的作用域
    {
        reg tmp_reg = 1;  // 请求一个寄存器
    }  // tmp_reg 会被释放
    reg tmp_reg;

    // 方法2，用 new 和 delete 分配和释放寄存器
    // 见基础用法

    // 方法3，重命名，不推荐
    reg new_reg = std::move(old_reg);  // 这相当于把 old_reg 重命名，值也会带走
    try {
        old_reg = A[0];  // 之后 old_reg 会失效，值会变为随机数，再使用它会抛出运行时错误
    } catch (InactiveRegisterException& e) {
        std::cout << "InactiveRegisterException: " << e.what() << std::endl;
    }
    std::cout << old_reg.info() << std::endl   // $4(INACTIVE): 1804289383
              << new_reg.info() << std::endl;  // $4(ACTIVE): 10
    new_reg = 1;
    std::cout << old_reg.info() << std::endl   // $4(INACTIVE): 1804289383
              << new_reg.info() << std::endl;  // $4(ACTIVE): 1
    /*****************************/


    /********** 怎么使用函数 **********/
    // 函数可能导致临时寄存器的创建和销毁，当你极限使用寄存器时可能会很抓狂，但当你寄存器比较富余时函数可以随便写
    // 我们认为一般人没必要写函数

    // 直接传参，默认是拷贝构造，意味着函数体内会多占用一个寄存器
    // void example0(ptr_reg ptr);
    
    // 如果这不是你的本意，我们传引用即可
    // void example1(ptr_reg& ptr);
    // example1(A);

    // 或者传地址（对于没学过 C++）的同学
    // void example1(ptr_reg* ptr);
    // example1(&A);

    // 或者使用 std::move，虽然这通常没必要，函数开始后 ptr 变成了 A，A 失效，函数结束后 ptr 销毁，A 依然失效。
    // void example2(ptr_reg ptr);
    // example2(std::move(A));

    // 函数返回值则会帮你创建一个临时寄存器，你可以观察到 max 变大了 1。你更应该传入一个结果寄存器，并将函数返回类型改为 void
    std::cout << "Non-function After\t" << "Max:" << get_max_reg_count()
              << "\tCurrent: " << get_current_reg_count() << std::endl;

    std::cout << "Before\t" << "Max:" << get_max_reg_count() << "\tCurrent: " << get_current_reg_count() << std::endl;
    // Before  Max:14  Current: 14
    a = example_return(A) + a;
    std::cout << "After\t" << "Max:" << get_max_reg_count() << "\tCurrent: " << get_current_reg_count() << std::endl;
    // After   Max:15  Current: 14
    /*****************************/


    /********** 一个说明 **********/
    // 这里有一个实现上故意放过的漏洞，
    // 对于二元计算，即两个操作数，一个结果的情况，我们是不需要临时寄存器的
    // 然而你可以写出非二元计算的复杂式子
    reg total;
    total = (b - a) * (c - a);  // without a temporary register
    // 我们不推荐你利用这个漏洞，你可以写成这样：
    reg tmp = b - a;  // actually, you need a temporary register
    reg total2 = c - a;
    total2 = tmp * total;
    // 但是，因为在 cachelab 中，助教没有想到可以利用这个漏洞的场景
    // 多数情况下你写出来的式子是可以被编译器优化成不需要临时寄存器的，所以我们为了多数情况的方便，保留了这个漏洞
    // 比如你可能会频繁地写 i * n + j，如果我们禁止了多元计算，你的代码会变得很丑陋
    /*****************************/
}

reg example_return(ptr_reg& ptr){
    std::cout << "In\t" << "Max:" << get_max_reg_count() << "\tCurrent: " << get_current_reg_count() << std::endl;
    // In      Max:14  Current: 14
    return ptr[0];
}

int main() {
    auto [A, B, C, buffer] = init(32, 32, 32);
    demo(std::move(A), std::move(B), std::move(C), std::move(buffer));
}

#else

int main() {
    std::cout << "Please refer to the non-explicit version of demo.cpp" << std::endl;
}

#endif