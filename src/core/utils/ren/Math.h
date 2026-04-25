#ifndef REN_MATH_H
#define REN_MATH_H

#include <cassert>
#include <cmath>
#include <vector>

#include "core/utils/ren/AlignedAllocator.h"
#include "core/utils/ren/MathImpl.h"

namespace REN {

    enum class ContainerType {
        Owning,
        View,
        ConstView
    };

    struct Matrix;
    struct MatrixView;

    template <ContainerType CT>
    struct VectorImpl;

    using Vector = VectorImpl<ContainerType::Owning>;
    using VectorView = VectorImpl<ContainerType::View>;
    using ConstVectorView = VectorImpl<ContainerType::ConstView>;

    Vector gmresRestarted(const Matrix& a, const Vector& b, size_t m, size_t maxRestarts, float tol);

    template <ContainerType CT>
    struct VectorImpl {
        private:
            friend struct VectorImpl<ContainerType::Owning>;
            friend struct VectorImpl<ContainerType::View>;
            friend struct VectorImpl<ContainerType::ConstView>;

            struct Pointer {
                float* ptr;

                constexpr Pointer(float* p) : ptr(p) {}

                constexpr float* operator*() const {
                    return ptr;
                }

                constexpr float* data() const {
                    return ptr;
                }
            };

            struct ConstPointer {
                const float* ptr;

                constexpr ConstPointer(const float* p) : ptr(p) {}

                constexpr const float* operator*() const {
                    return ptr;
                }

                constexpr const float* data() const {
                    return ptr;
                }
            };

        public:

        typedef std::conditional_t<CT == ContainerType::Owning, std::vector<float, AlignedAllocator<float, REQUIRED_ALIGNMENT>>,
            std::conditional_t<CT == ContainerType::View, Pointer, ConstPointer>> StorageType;

        StorageType x;
        size_t size;

        inline VectorImpl(size_t s) requires(CT == ContainerType::Owning) :
            x(s, 0.0f), size(s) {}
        
        inline VectorImpl(float* data, size_t s) requires(CT == ContainerType::View) :
            x(data), size(s) {}

        inline VectorImpl(const float* data, size_t s) requires(CT == ContainerType::ConstView) :
            x(data), size(s) {}

        template <ContainerType otherCT>
        inline VectorImpl(const VectorImpl<otherCT>& other)
            requires(!(otherCT == ContainerType::ConstView && CT == ContainerType::View)) {
            if constexpr (CT == ContainerType::Owning) {
                x.resize(other.size);
                __unsafe_copy(x.data(), other.x.data(), other.size);
            } else
                x = other.x.data();

            size = other.size;
        }

        template <ContainerType otherCT>
        inline VectorImpl(VectorImpl&& other)
            requires(CT == otherCT || CT == ContainerType::Owning ||
                (CT == ContainerType::View && otherCT != ContainerType::ConstView)) {

            if constexpr(CT == otherCT)
                x = std::move(other.x);
            else if constexpr(CT == ContainerType::Owning) {
                x.resize(other.size);
                __unsafe_copy(x.data(), other.x.data(), other.size);
            } else
                x = other.x.data();

            size = other.size;
        }

        template <ContainerType otherCT>
        inline VectorImpl& operator=(const VectorImpl<otherCT>& other)
            requires(!(otherCT == ContainerType::ConstView && CT == ContainerType::View) &&
                     CT != ContainerType::ConstView) {
            if constexpr (CT == otherCT) {
                if(this == &other)
                    return *this;
            }

            if constexpr (CT == ContainerType::Owning)
                x.resize(other.size);

            __unsafe_copy(x.data(), other.x.data(), other.size);

            size = other.size;
            return *this;
        }

        template <ContainerType otherCT>
        inline VectorImpl& operator=(VectorImpl<otherCT>&& other)
            requires(CT == otherCT) {

            if(this == &other)
                return *this;

            x = std::move(other.x);
            size = other.size;

            return *this;
        }

        inline float operator()(size_t idx) const {
            assert(idx < size);
            return x[idx];
        }

        inline float& operator()(size_t idx) {
            assert(idx < size);
            return x[idx];
        }

        template <ContainerType otherCT>
        inline Vector operator+(const VectorImpl<otherCT>& other) const {
            assert(size == other.size);
            Vector result(size);
            if(this->x.data() == other.x.data())
                __unsafe_mul(result.x.data(), x.data(), 2.0f, size);
            else
                __unsafe_add(result.x.data(), x.data(), other.x.data(), size);

            return result;
        }

        template <ContainerType otherCT>
        inline VectorImpl& operator+=(const VectorImpl<otherCT>& other) {
            assert(size == other.size);
            if(this->x.data() == other.x.data())
                __unsafe_mul_self(x.data(), 2.0f, size);
            else
                __unsafe_add_self(x.data(), other.x.data(), size);

            return *this;
        }

        template <ContainerType otherCT>
        inline Vector operator-(const VectorImpl<otherCT>& other) const {
            assert(size == other.size);
            Vector result(size);
            if(this->x.data() != other.x.data())
                __unsafe_sub(result.x.data(), x.data(), other.x.data(), size);

            // Ergebnis ist 0 und Vektoren werden mit 0 initialisiert (wenn this == &other)

            return result;
        }

        template <ContainerType otherCT>
        inline VectorImpl& operator-=(const VectorImpl<otherCT>& other) {
            assert(size == other.size);
            if(this->x.data() == other.x.data())
                __unsafe_set_zero(x.data(), size);
            else
                __unsafe_sub_self(x.data(), other.x.data(), size);

            return *this;
        }

        inline Vector operator*(float scalar) const {
            Vector result(size);
            __unsafe_mul(result.x.data(), x.data(), scalar, size);
            return result;
        }

        friend inline Vector operator*(float scalar, const VectorImpl& vec) {
            return vec * scalar;
        }

        inline VectorImpl& operator*=(float scalar) {
            __unsafe_mul_self(x.data(), scalar, size);
            return *this;
        }

        inline Vector operator/(float scalar) const {
            Vector result(size);
            __unsafe_div(result.x.data(), x.data(), scalar, size);
            return result;
        }

        inline VectorImpl& operator/=(float scalar) {
            __unsafe_div_self(x.data(), scalar, size);
            return *this;
        }

        template <ContainerType otherCT>
        inline float dot(const VectorImpl<otherCT>& other) const {
            assert(size == other.size);
            if(this->x.data() == other.x.data())
                return __unsafe_norm_sq(x.data(), size);
            else
                return __unsafe_dot(x.data(), other.x.data(), size);
        }

        inline float normSq() const {
            return __unsafe_norm_sq(x.data(), size);
        }

        inline float norm() const {
            return std::sqrt(normSq());
        }

    };

    struct Matrix {
        std::vector<float, AlignedAllocator<float, REQUIRED_ALIGNMENT>> w;
        size_t cols, rows;

        inline Matrix(size_t s) : w(s * s, 0.0f), cols(s), rows(s) {}
        inline Matrix(size_t c, size_t r) : w(c * r, 0.0f), cols(c), rows(r) {}

        inline float operator()(size_t c, size_t r) const {
            assert(c < cols && r < rows);
            return w[r * cols + c];
        }

        inline float& operator()(size_t c, size_t r) {
            assert(c < cols && r < rows);
            return w[r * cols + c];
        }

        inline VectorView operator()(size_t c) {
            assert(c < cols);
            return VectorView(w.data() + c * rows, rows);
        }

        inline ConstVectorView operator()(size_t c) const {
            assert(c < cols);
            return ConstVectorView(w.data() + c * rows, rows);
        }

        inline Matrix operator+(const Matrix& other) const {
            assert(cols == other.cols && rows == other.rows);
            Matrix result(cols, rows);
            if(this == &other)
                __unsafe_mul(result.w.data(), w.data(), 2.0f, cols * rows);
            else
                __unsafe_add(result.w.data(), w.data(), other.w.data(), cols * rows);

            return result;
        }

        inline Matrix& operator+=(const Matrix& other) {
            assert(cols == other.cols && rows == other.rows);
            if(this == &other)
                __unsafe_mul_self(w.data(), 2.0f, cols * rows);
            else
                __unsafe_add_self(w.data(), other.w.data(), cols * rows);

            return *this;
        }

        inline Matrix operator-(const Matrix& other) const {
            assert(cols == other.cols && rows == other.rows);
            Matrix result(cols, rows);
            if(this != &other)
                __unsafe_sub(result.w.data(), w.data(), other.w.data(), cols * rows);

            // Ergebnis ist 0 und Matrizen werden mit 0 initialisiert (wenn this == &other)

            return result;
        }

        inline Matrix& operator-=(const Matrix& other) {
            assert(cols == other.cols && rows == other.rows);
            if(this == &other)
                __unsafe_set_zero(w.data(), cols * rows);
            else
                __unsafe_sub_self(w.data(), other.w.data(), cols * rows);

            return *this;
        }

        inline Matrix operator*(float scalar) const {
            Matrix result(cols, rows);
            __unsafe_mul(result.w.data(), w.data(), scalar, cols * rows);
            return result;
        }

        friend inline Matrix operator*(float scalar, const Matrix& mat) {
            return mat * scalar;
        }

        inline Matrix& operator*=(float scalar) {
            __unsafe_mul_self(w.data(), scalar, cols * rows);
            return *this;
        }

        inline Matrix operator/(float scalar) const {
            Matrix result(cols, rows);
            __unsafe_div(result.w.data(), w.data(), scalar, cols * rows);
            return result;
        }

        inline Matrix& operator/=(float scalar) {
            __unsafe_div_self(w.data(), scalar, cols * rows);
            return *this;
        }

        template <ContainerType otherCT>
        inline Vector operator*(const VectorImpl<otherCT>& vec) const {
            assert(cols == vec.size);
            Vector result(rows);
            __unsafe_gemv(result.x.data(), w.data(), vec.x.data(), rows, cols);
            return result;
        }
    };
};

#endif