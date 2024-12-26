#pragma once
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#define NDEBUG

#ifdef USE_EXPLICIT
#define EXPLICIT_HINT explicit
#else
#define EXPLICIT_HINT
#endif

template <typename T>
class RegisterWarpper;

template <typename T>
class MemoryWarpper;

template <typename T>
class PtrWarpper;

template <typename T>
class BaseRegisterWarpper;

enum class MemoryAccessType {
    UNKOWN = 0,
    READ,
    WRITE,
    READ_WRITE
};

enum class RegisterWarpperState {
    ACTIVE,
    INACTIVE,
    TMP  // deprecated
};

class CachelabException : public std::exception {
   public:
    CachelabException(const char* msg)
        : msg_(msg) {}
    const char* what() const noexcept override {
        return msg_;
    }

   private:
    const char* msg_;
};

class MemoryDataCalculationException : public CachelabException {
   public:
    MemoryDataCalculationException(const char* msg = "you can't directly calculate the data in memory")
        : CachelabException(msg) {}
};

class InactiveRegisterException : public CachelabException {
   public:
    InactiveRegisterException(const char* msg = "you can't operate on an inactive register")
        : CachelabException(msg) {}
};

class MemoryToMemoryAssignmentException : public CachelabException {
   public:
    MemoryToMemoryAssignmentException(const char* msg = "you can't assign a memory to another memory directly")
        : CachelabException(msg) {}
};

class OutOfRegistersException : public CachelabException {
   public:
    OutOfRegistersException(const char* msg = "no more available registers")
        : CachelabException(msg) {}
};

inline int max_reg_count = 0;
inline int current_reg_count = 0;

namespace {
constexpr int reg_num = 32;

bool reg_map[reg_num] = {0};

inline int find_reg() {
    for (int i = 0; i < reg_num; i++) {
        if (!reg_map[i]) {
            reg_map[i] = true;
#ifndef NDEBUG
            std::cerr << "allocate reg: " << i << std::endl;
#endif
            current_reg_count++;
            max_reg_count = std::max(max_reg_count, current_reg_count);
            return i;
        }
    }
    throw OutOfRegistersException();
}

inline void free_reg(int reg_id) {
#ifndef NDEBUG
    std::cerr << "free reg: " << reg_id << std::endl;
#endif
    reg_map[reg_id] = false;
    current_reg_count--;
}

}  // namespace

template <typename T>
class BaseRegisterWarpper {
   protected:
    T reg_;
    RegisterWarpperState state_;
    int reg_id_;

    // you can't set state directly
    BaseRegisterWarpper(T reg, RegisterWarpperState state, int reg_id = -2)
        : reg_(reg), state_(state), reg_id_(reg_id) {
        if (state == RegisterWarpperState::ACTIVE) {
            reg_id_ = find_reg();
        }
    }

   public:
    EXPLICIT_HINT BaseRegisterWarpper(T reg = 0)
        : reg_(reg), state_(RegisterWarpperState::ACTIVE), reg_id_(find_reg()) {
    }

    EXPLICIT_HINT BaseRegisterWarpper(const MemoryWarpper<T>& other)
        : reg_(*other.ptr_), state_(RegisterWarpperState::ACTIVE), reg_id_(find_reg()) {
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::READ, other.ptr_, reg_id_});
    }

    EXPLICIT_HINT BaseRegisterWarpper(const MemoryWarpper<T>&& other)
        : reg_(*other.ptr_), state_(RegisterWarpperState::ACTIVE), reg_id_(find_reg()) {
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::READ, other.ptr_, reg_id_});
    }

    EXPLICIT_HINT BaseRegisterWarpper(const BaseRegisterWarpper<T>& other)
        : reg_(other.reg_), state_(RegisterWarpperState::ACTIVE), reg_id_(find_reg()) {
    }

    EXPLICIT_HINT BaseRegisterWarpper(BaseRegisterWarpper<T>&& other)
        : reg_(other.reg_), state_(other.state_), reg_id_(other.reg_id_) {
        if (other.state_ == RegisterWarpperState::ACTIVE) {
            other.state_ = RegisterWarpperState::INACTIVE;
            other.reg_ = rand();
        } else if (other.state_ == RegisterWarpperState::INACTIVE) {
            throw InactiveRegisterException();
        }
    }

    operator int() const {
        return reg_;
    }

    ~BaseRegisterWarpper() {
        if (state_ == RegisterWarpperState::ACTIVE) {
            free_reg(reg_id_);
        }
    }

    void check_valid() const {
        if (state_ != RegisterWarpperState::ACTIVE) {
            throw InactiveRegisterException();
        }
    }

    BaseRegisterWarpper<T>& operator=(T other) {
        check_valid();
        reg_ = other;
        return *this;
    }

    BaseRegisterWarpper<T>& operator=(const BaseRegisterWarpper<T>& other) {
        check_valid();
        reg_ = other.reg_;
        return *this;
    }

    BaseRegisterWarpper<T>& operator=(BaseRegisterWarpper<T>&& other) {
        // TODO:
        check_valid();
        reg_ = other.reg_;
        if (other.state_ == RegisterWarpperState::ACTIVE) {
            other.state_ = RegisterWarpperState::INACTIVE;
        }
        return *this;
    }

    BaseRegisterWarpper<T>& operator=(const MemoryWarpper<T>& other) {
        check_valid();
        reg_ = *(other.ptr_);
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::READ, other.ptr_, reg_id_});
        return *this;
    }

    BaseRegisterWarpper<T>& operator=(const MemoryWarpper<T>&& other) {
        check_valid();
        reg_ = *(other.ptr_);
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::READ, other.ptr_, reg_id_});
        return *this;
    }

    /* <=> */
    bool operator<(const BaseRegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ < other.reg_;
    }
    bool operator<(const BaseRegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ < other.reg_;
    }
    bool operator<(const T& other) const {
        check_valid();
        return reg_ < other;
    }
    bool operator<(const T&& other) const {
        check_valid();
        return reg_ < other;
    }

    bool operator>(const BaseRegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ > other.reg_;
    }
    bool operator>(const BaseRegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ > other.reg_;
    }
    bool operator>(const T& other) const {
        check_valid();
        return reg_ > other;
    }
    bool operator>(const T&& other) const {
        check_valid();
        return reg_ > other;
    }

    bool operator<=(const BaseRegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ <= other.reg_;
    }
    bool operator<=(const BaseRegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ <= other.reg_;
    }
    bool operator<=(const T& other) const {
        check_valid();
        return reg_ <= other;
    }
    bool operator<=(const T&& other) const {
        check_valid();
        return reg_ <= other;
    }

    bool operator>=(const BaseRegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ >= other.reg_;
    }
    bool operator>=(const BaseRegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ >= other.reg_;
    }
    bool operator>=(const T& other) const {
        check_valid();
        return reg_ >= other;
    }
    bool operator>=(const T&& other) const {
        check_valid();
        return reg_ >= other;
    }

    bool operator==(const BaseRegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ == other.reg_;
    }
    bool operator==(const BaseRegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ == other.reg_;
    }
    bool operator==(const T& other) const {
        check_valid();
        return reg_ == other;
    }
    bool operator==(const T&& other) const {
        check_valid();
        return reg_ == other;
    }

    friend std::ostream& operator<<(std::ostream& os, const RegisterWarpper<T>& reg) {
        os << reg.reg_;
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const RegisterWarpper<T>&& reg) {
        os << reg.reg_;
        return os;
    }

    std::string info() const {
        std::string state;
        switch (state_) {
            case RegisterWarpperState::ACTIVE:
                state = "ACTIVE";
                break;
            case RegisterWarpperState::INACTIVE:
                state = "INACTIVE";
                break;
            case RegisterWarpperState::TMP:
                state = "TMP";
                break;
            default:
                state = "UNKOWN";
                break;
        }
        return "$" + std::to_string(reg_id_) + "(" + state + "): " + std::to_string(reg_);
    }

    friend class MemoryWarpper<T>;
    // friend class PtrWarpper<T>;
};

template <typename T>
class MemoryAccessLog {
   public:
    MemoryAccessType type_;
    T* addr_;
    int reg_id_;
};

/************************************************************************************/

template <typename T>
class PtrWarpper : public BaseRegisterWarpper<int> {
    // 本来 PtrWarpper 设计是不占用寄存器的，后来决定改成占用一个寄存器
    // 为了不修改原有代码，我们只借用 BaseRegisterWarpper<int> 的构造和析构，以让他占用一个寄存器，而不真的使用它
   public:
    static std::vector<MemoryAccessLog<T>> access_logs;
    static T* base;
    static T* base_offset;
    T* ptr_;
    PtrWarpper(T* ptr)
        : BaseRegisterWarpper(), ptr_(ptr) {
    }

    using BaseRegisterWarpper<int>::operator=;
    using BaseRegisterWarpper<int>::operator==;
    using BaseRegisterWarpper<int>::operator<;
    using BaseRegisterWarpper<int>::operator>;
    using BaseRegisterWarpper<int>::operator<=;
    using BaseRegisterWarpper<int>::operator>=;

    MemoryWarpper<T> operator*() const {
        return MemoryWarpper<T>(ptr_);
    }
    MemoryWarpper<T> operator[](int offset) const {
        return MemoryWarpper<T>(ptr_ + offset);
    }
    MemoryWarpper<T> operator[](const RegisterWarpper<T>& offset) const {
        return MemoryWarpper<T>(ptr_ + offset.reg_);
    }
    MemoryWarpper<T> operator[](const RegisterWarpper<T>&& offset) const {
        return MemoryWarpper<T>(ptr_ + offset.reg_);
    }

    PtrWarpper<T> operator+(int offset) const {
        return PtrWarpper<T>(ptr_ + offset);
    }
    PtrWarpper<T> operator+(const RegisterWarpper<T>& offset) const {
        return PtrWarpper<T>(ptr_ + offset.reg_);
    }
    PtrWarpper<T> operator+(const RegisterWarpper<T>&& offset) const {
        return PtrWarpper<T>(ptr_ + offset.reg_);
    }

    PtrWarpper<T> operator-(int offset) const {
        return PtrWarpper<T>(ptr_ - offset);
    }
    PtrWarpper<T> operator-(const RegisterWarpper<T>& offset) const {
        return PtrWarpper<T>(ptr_ - offset.reg_);
    }
    PtrWarpper<T> operator-(const RegisterWarpper<T>&& offset) const {
        return PtrWarpper<T>(ptr_ - offset.reg_);
    }

    T operator-(PtrWarpper<T> other) const {
        return ptr_ - other.ptr_;
    }

    PtrWarpper<T> operator++() {
        ptr_++;
        return *this;
    }
    // don't implement this
    // PtrWarpper<T> operator++(int) {
    // }

    PtrWarpper<T> operator--() {
        ptr_--;
        return *this;
    }
    // don't implement this
    // PtrWarpper<T> operator--(int) {
    // }

    PtrWarpper<T> operator+=(int offset) {
        ptr_ += offset;
        return *this;
    }
    PtrWarpper<T> operator+=(const RegisterWarpper<T>& offset) {
        ptr_ += offset.reg_;
        return *this;
    }
    PtrWarpper<T> operator+=(const RegisterWarpper<T>&& offset) {
        ptr_ += offset.reg_;
        return *this;
    }

    PtrWarpper<T> operator-=(int offset) {
        ptr_ -= offset;
        return *this;
    }
    PtrWarpper<T> operator-=(const RegisterWarpper<T>& offset) {
        ptr_ -= offset.reg_;
        return *this;
    }
    PtrWarpper<T> operator-=(const RegisterWarpper<T>&& offset) {
        ptr_ -= offset.reg_;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const PtrWarpper<T>& ptr) {
        os << ptr.ptr_ - PtrWarpper<T>::base + PtrWarpper<T>::base_offset;
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const PtrWarpper<T>&& ptr) {
        os << ptr.ptr_ - PtrWarpper<T>::base + PtrWarpper<T>::base_offset;
        return os;
    }

    std::string info() const {
        std::string state;
        switch (state_) {
            case RegisterWarpperState::ACTIVE:
                state = "ACTIVE";
                break;
            case RegisterWarpperState::INACTIVE:
                state = "INACTIVE";
                break;
            case RegisterWarpperState::TMP:
                state = "TMP";
                break;
            default:
                state = "UNKOWN";
                break;
        }
        return "$" + std::to_string(reg_id_) + "(" + state + "): " + std::to_string(reinterpret_cast<size_t>(ptr_ - PtrWarpper<T>::base + PtrWarpper<T>::base_offset));
    }
};

template <typename T>
std::vector<MemoryAccessLog<T>> PtrWarpper<T>::access_logs;

template <typename T>
T* PtrWarpper<T>::base = 0;

template <typename T>
T* PtrWarpper<T>::base_offset = 0;

template <typename T>
class MemoryWarpper {
   public:
    T* ptr_;
    explicit MemoryWarpper(T* ptr)
        : ptr_(ptr) {}
    explicit MemoryWarpper(const MemoryWarpper& other) = delete;
    explicit MemoryWarpper(MemoryWarpper&& other) = delete;

    const T& operator=(const T& other) {
        *ptr_ = other;
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::WRITE, ptr_, -1});
        return other;
    }
    const T& operator=(const T&& other) {
        *ptr_ = other;
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::WRITE, ptr_, -1});
        return other;
    }

    const RegisterWarpper<T>& operator=(const RegisterWarpper<T>& other) {
        *ptr_ = other.reg_;
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::WRITE, ptr_, other.reg_id_});
        return other;
    }
    const RegisterWarpper<T>& operator=(const RegisterWarpper<T>&& other) {
        *ptr_ = other.reg_;
        PtrWarpper<T>::access_logs.push_back({MemoryAccessType::WRITE, ptr_, other.reg_id_});
        return other;
    }

    friend std::ostream& operator<<(std::ostream& os, const MemoryWarpper<T>& mem) {
        os << *mem.ptr_;
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const MemoryWarpper<T>&& mem) {
        os << *mem.ptr_;
        return os;
    }
};

template <typename T>
class RegisterWarpper : public BaseRegisterWarpper<int> {
    static_assert(std::is_same<T, int>::value, "T must be int, may be fixed in the future");

   private:
    // you can't set state directly
    RegisterWarpper(T reg, RegisterWarpperState state, int reg_id = -2)
        : BaseRegisterWarpper<T>(reg, state, reg_id) {
    }

   public:
    using BaseRegisterWarpper<T>::BaseRegisterWarpper;
    using BaseRegisterWarpper<T>::operator=;
    using BaseRegisterWarpper<T>::operator==;
    using BaseRegisterWarpper<T>::operator<;
    using BaseRegisterWarpper<T>::operator>;
    using BaseRegisterWarpper<T>::operator<=;
    using BaseRegisterWarpper<T>::operator>=;

    /* + */

    T operator+(const RegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ + other.reg_;
    }
    T operator+(const RegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ + other.reg_;
    }
    T operator+(const T other) const {
        check_valid();
        return reg_ + other;
    }
    friend T operator+(const T other, const RegisterWarpper<T>& reg) {
        reg.check_valid();
        return other + reg.reg_;
    }
    T operator+(const MemoryWarpper<T>& other) const = delete;
    T operator+(const MemoryWarpper<T>&& other) const = delete;

    T operator+=(const RegisterWarpper<T>& other) {
        check_valid();
        other.check_valid();
        reg_ += other.reg_;
        return reg_;
    }
    T operator+=(const RegisterWarpper<T>&& other) {
        check_valid();
        other.check_valid();
        reg_ += other.reg_;
        return reg_;
    }
    T operator+=(const T other) {
        check_valid();
        reg_ += other;
        return reg_;
    }
    T operator+=(const MemoryWarpper<T>& other) = delete;
    T operator+=(const MemoryWarpper<T>&& other) = delete;

    T operator++() {
        check_valid();
        reg_++;
        return reg_;
    }

    // don't implement this
    // RegisterWarpper operator++(int) {
    // }

    /* - */

    T operator-(const RegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ - other.reg_;
    }
    T operator-(const RegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ - other.reg_;
    }
    T operator-(const T other) const {
        check_valid();
        other.check_valid();
        return reg_ - other;
    }
    friend T operator-(const T other, const RegisterWarpper<T>& reg) {
        reg.check_valid();
        return other - reg.reg_;
    }
    T operator-(const MemoryWarpper<T>& other) const = delete;
    T operator-(const MemoryWarpper<T>&& other) const = delete;

    T operator-=(const RegisterWarpper<T>& other) {
        check_valid();
        other.check_valid();
        reg_ -= other.reg_;
        return reg_;
    }
    T operator-=(const RegisterWarpper<T>&& other) {
        check_valid();
        other.check_valid();
        reg_ -= other.reg_;
        return reg_;
    }
    T operator-=(const MemoryWarpper<T>& other) const = delete;
    T operator-=(const MemoryWarpper<T>&& other) const = delete;
    T operator-=(const T other) {
        check_valid();
        reg_ -= other;
        return reg_;
    }
    T operator--() {
        check_valid();
        reg_--;
        return reg_;
    }

    // don't implement this
    // RegisterWarpper operator--(int) {
    // }

    /* * */
    T operator*(const RegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ * other.reg_;
    }
    T operator*(const RegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ * other.reg_;
    }
    T operator*(const T other) const {
        check_valid();
        return reg_ * other;
    }
    friend T operator*(const T other, const RegisterWarpper<T>& reg) {
        reg.check_valid();
        return other * reg.reg_;
    }
    T operator*(const MemoryWarpper<T>& other) const = delete;
    T operator*(const MemoryWarpper<T>&& other) const = delete;

    T operator*=(const RegisterWarpper<T>& other) {
        check_valid();
        other.check_valid();
        reg_ *= other.reg_;
        return reg_;
    }
    T operator*=(const RegisterWarpper<T>&& other) {
        check_valid();
        other.check_valid();
        reg_ *= other.reg_;
        return reg_;
    }
    T operator*=(const T other) {
        check_valid();
        reg_ *= other;
        return reg_;
    }
    T operator*=(const MemoryWarpper<T>& other) const = delete;
    T operator*=(const MemoryWarpper<T>&& other) const = delete;

    /* / */
    T operator/(const RegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ / other.reg_;
    }
    T operator/(const RegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ / other.reg_;
    }
    T operator/(const T other) const {
        check_valid();
        return reg_ / other;
    }
    friend T operator/(const T other, const RegisterWarpper<T>& reg) {
        reg.check_valid();
        return other / reg.reg_;
    }
    T operator/(const MemoryWarpper<T>& other) const = delete;
    T operator/(const MemoryWarpper<T>&& other) const = delete;

    T operator/=(const RegisterWarpper<T>& other) {
        check_valid();
        other.check_valid();
        reg_ /= other.reg_;
        return reg_;
    }
    T operator/=(const RegisterWarpper<T>&& other) {
        check_valid();
        other.check_valid();
        reg_ /= other.reg_;
        return reg_;
    }
    T operator/=(const T other) {
        check_valid();
        reg_ /= other;
        return reg_;
    }
    T operator/=(const MemoryWarpper<T>& other) const = delete;
    T operator/=(const MemoryWarpper<T>&& other) const = delete;

    /* % */
    T operator%(const RegisterWarpper<T>& other) const {
        check_valid();
        other.check_valid();
        return reg_ % other.reg_;
    }
    T operator%(const RegisterWarpper<T>&& other) const {
        check_valid();
        other.check_valid();
        return reg_ % other.reg_;
    }
    T operator%(const T other) const {
        check_valid();
        return reg_ % other;
    }
    friend T operator%(const T other, const RegisterWarpper<T>& reg) {
        reg.check_valid();
        return other % reg.reg_;
    }
    T operator%(const MemoryWarpper<T>& other) const = delete;
    T operator%(const MemoryWarpper<T>&& other) const = delete;

    T operator%=(const RegisterWarpper<T>& other) {
        check_valid();
        other.check_valid();
        reg_ %= other.reg_;
        return reg_;
    }
    T operator%=(const RegisterWarpper<T>&& other) {
        check_valid();
        other.check_valid();
        reg_ %= other.reg_;
        return reg_;
    }
    T operator%=(const T other) {
        check_valid();
        reg_ %= other;
        return reg_;
    }
    T operator%=(const MemoryWarpper<T>& other) const = delete;
    T operator%=(const MemoryWarpper<T>&& other) const = delete;

    friend class MemoryWarpper<T>;
    friend class PtrWarpper<T>;
};

using reg = RegisterWarpper<int>;
using ptr_reg = PtrWarpper<int>;

inline int get_max_reg_count() {
    return max_reg_count;
}

inline int get_current_reg_count() {
    return current_reg_count;
}

inline void print_log() {
    for (auto& log : ptr_reg::access_logs) {
        switch (log.type_) {
            case MemoryAccessType::READ:
                std::cout << " L ";
                break;
            case MemoryAccessType::WRITE:
                std::cout << " S ";
                break;
            case MemoryAccessType::READ_WRITE:
                std::cout << " M ";
                break;
            default:
                throw std::runtime_error("unkown memory access type");
        }
        std::cout << std::noshowbase << std::hex << log.addr_ - ptr_reg::base + ptr_reg::base_offset;
        std::cout << std::dec << ",4 " << log.reg_id_ << std::endl;
    }
}