#include "core/utils/ren/Math.h"

#include <tuple>

using namespace REN;

std::tuple<Vector, float> gmres(const Matrix& a, const Vector& b, Vector x0, size_t maxIter, float tol, float eps = 1e-8f) {
    if(b.size != a.cols)
        throw std::invalid_argument("Matrix and vector size must match.");

    size_t size = b.size;

    Vector r0(size);
    r0 = b - a * x0;
    float beta = r0.norm();

    // Abbruch, wenn die Anfangsapproximation bereits eine gute Lösung ist
    if(beta <= tol)
        return std::make_tuple(x0, beta);

    // Orthonormale Basis des Krylov-Unterraums
    const size_t stride = maxIter + 1;
    Matrix q(stride, size);
    q(0) = r0 / beta;
    float* pq = std::assume_aligned<REQUIRED_ALIGNMENT>(q.w.data());

    // Hessenberg-Matrix im Column-Major-Layout
    Matrix h(maxIter, stride);
    float* __restrict ph = std::assume_aligned<REQUIRED_ALIGNMENT>(h.w.data());

    // Givens-Rotationen
    Vector cs(maxIter);
    float* __restrict pcs = std::assume_aligned<REQUIRED_ALIGNMENT>(cs.x.data());
    Vector sn(maxIter);
    float* __restrict psn = std::assume_aligned<REQUIRED_ALIGNMENT>(sn.x.data());

    // Rechte Seite des reduzierten Problems
    Vector g(maxIter + 1);
    float* __restrict pg = std::assume_aligned<REQUIRED_ALIGNMENT>(g.x.data());
    pg[0] = beta;

    size_t convergedIter = maxIter;
    for(size_t iter = 0; iter < maxIter; iter++) {

        // Arnoldi-Iteration
        Vector w(size);
        float* pw = std::assume_aligned<REQUIRED_ALIGNMENT>(w.x.data());

        for(size_t i = 0; i < size; i++)
            pw[i] = a(i).dot(q(iter));

        for(size_t j = 0; j <= iter; j++) {
            float h_ij = w.dot(q(j));
            w -= h_ij * q(j);
            ph[iter * stride + j] = h_ij;
        }

        float h_next = w.norm();
        ph[iter * stride + iter + 1] = h_next;

        // Givens-Rotationen anwenden

        // Wende alte Rotationen auf die neue Spalte der Hessenberg-Matrix an
        for (size_t j = 0; j < iter; j++) {
            float temp = pcs[j] * ph[iter * stride + j] + psn[j] * ph[iter * stride + j + 1];
            ph[iter * stride + j + 1] = -psn[j] * ph[iter * stride + j] + pcs[j] * ph[iter * stride + j + 1];
            ph[iter * stride + j] = temp;
        }

        // Berechne die neue Givens-Rotation, um h[iter * stride + iter] zu eliminieren
        float h_diag = ph[iter * stride + iter];
        float h_subdiag = ph[iter * stride + iter + 1];
        float r = std::hypot(h_diag, h_subdiag);

        if(r == 0.0f) {
            pcs[iter] = 1.0f;
            psn[iter] = 0.0f;
        } else {
            pcs[iter] = h_diag / r;
            psn[iter] = h_subdiag / r;
        }

        // Wende die neue Givens-Rotation an
        ph[iter * stride + iter] = pcs[iter] * h_diag + psn[iter] * h_subdiag;
        ph[iter * stride + iter + 1] = 0.0f;

        pg[iter + 1] = -psn[iter] * pg[iter];
        pg[iter] = pcs[iter] * pg[iter];

        // Konvergenz prüfen
        float residual = std::abs(pg[iter + 1]);
        if(h_next < eps || residual <= tol) {
            convergedIter = iter + 1;
            break;
        }

        q(iter + 1) = w / h_next;
    }

    // Rückwärtssubstitution
    Vector y(convergedIter);
    float* __restrict py = std::assume_aligned<REQUIRED_ALIGNMENT>(y.x.data());
    for(int32_t i = (int32_t)(convergedIter - 1); i >= 0; i--) {
        float sum = pg[i];
        for(size_t j = i + 1; j < convergedIter; j++)
            sum -= ph[j * stride + i] * py[j];

        py[i] = sum / ph[i * stride + i];
    }

    // Berechnung der Lösung x = x0 + Q * y
    float* __restrict px0 = std::assume_aligned<REQUIRED_ALIGNMENT>(x0.x.data());
    Vector x(size);
    float* __restrict px = std::assume_aligned<REQUIRED_ALIGNMENT>(x.x.data());
    for(size_t i = 0; i < size; i++) {
        float sum = 0.0f;
        for(size_t j = 0; j < convergedIter; j++)
            sum += pq[j * size + i] * py[j];

        px[i] = px0[i] + sum;
    }

    return std::make_tuple(x, std::abs(pg[convergedIter]));
}

Vector REN::gmresRestarted(const Matrix& a, const Vector& b, size_t m, size_t maxRestarts, float tol) {
    // Anfangsapproximation x0 = b
    Vector x = b;

    float residual = 0.0f;
    for(size_t i = 0; i < maxRestarts; i++) {
        std::tie(x, residual) = gmres(a, b, x, m, tol);
        if(residual <= tol)
            break;
    }

    assert(residual <= tol);

    return x;
}