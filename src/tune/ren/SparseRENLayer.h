#ifndef REN_SPARSE_REN_LAYER_H
#define REN_SPARSE_REN_LAYER_H

#include <stdexcept>
#include <stddef.h>

#include "tune/ml/Math.h"

namespace REN {
    class SparseRENLayer {
        public:
            struct SurrogateWeights {
                std::vector<ML::Matrix> q;
                ML::Vector gammaRaw;
                size_t size;

                inline SurrogateWeights(size_t size) :
                    q(size, ML::Matrix(size)), gammaRaw(size * size), size(size) {}
            };

            struct Gradients {
                std::vector<ML::Matrix> q;
                ML::Vector gammaRaw;
                ML::Vector bias;
                ML::Vector inputGrad;
                size_t size;

                inline Gradients(size_t size) :
                    q(size, ML::Matrix(size)), gammaRaw(size * size),
                    bias(size * size), inputGrad(size * size), size(size) {}
            };

            struct ForwardResult {
                ML::Vector h_opt;
                ML::Vector z_opt;
                size_t iterations;
                float residual;

                inline ForwardResult(size_t s) : h_opt(s), z_opt(s), iterations(0), residual(0.0f) {}
                inline ForwardResult(const ML::Vector& h) :
                    h_opt(h), z_opt(h.size), iterations(0), residual(0.0f) {}
            };

            SurrogateWeights surrogateWeights;
            std::vector<ML::Matrix> transform;
            ML::Vector bias;
            size_t size;
            float epsilon;

            /**
             * @brief Konstruiert W_r anhand von W_q und gamma_raw.
             * W_r = -Q Q^T + diag((1 - epsilon) * sigmoid(gamma_raw))
             */
            void constructTransform();

            /**
             * @brief Konstruiert W_r für einen einzelnen Block anhand von W_q und gamma_raw.
             * W_r = -Q Q^T + diag((1 - epsilon) * sigmoid(gamma_raw))
             */
            void constructTransformBlock(size_t block);

            /**
             * @brief Normalisiert W_r so, dass der Spektralradius unter einem gegebenen Schwellenwert liegt.
             * 
             * @param maxSpectralRadius Der maximale erlaubte Spektralradius.
             */
            void normalize(float maxSpectralRadius);

            /**
             * @brief Berechnet die Spektralradius von W_r.
             * 
             * @param maxIterations Maximale Anzahl an Iterationen für die Power-Iteration.
             * @param tol Toleranz für die Konvergenz der Power-Iteration.
             */
            float spectralRadius(size_t maxIterations = 400, float tol = 1e-5f) const;

            /**
             * @brief Berechnet die Spektralradius eines einzelnen Blocks von W_r.
             * 
             * @param block Der Index des Blocks, für den der Spektralradius berechnet werden soll.
             * @param maxIterations Maximale Anzahl an Iterationen für die Power-Iteration.
             * @param tol Toleranz für die Konvergenz der Power-Iteration.
             */
            float blockSpectralRadius(size_t block, size_t maxIterations = 400, float tol = 1e-5f) const;

            inline SparseRENLayer(size_t size, float eps = 0.01) :
                surrogateWeights(size), transform(size, ML::Matrix(size)),
                bias(size * size), size(size), epsilon(eps) {

                constructTransform();
            }

            inline SparseRENLayer(const SurrogateWeights& mw, const std::vector<ML::Matrix>& rt, const ML::Vector& b, float eps = 0.01) :
                surrogateWeights(mw), transform(rt), bias(b), size(mw.size), epsilon(eps) {

                // Konsistenzprüfung
                if(rt[0].innerDim != size || rt[0].outerDim != size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and transform matrix.");

                if(b.size != size * size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and bias vector.");
            }

            inline SparseRENLayer(const SurrogateWeights& mw, float eps = 0.01) :
                surrogateWeights(mw), transform(mw.size, ML::Matrix(mw.size)), bias(mw.size * mw.size),
                size(mw.size), epsilon(eps) {

                constructTransform();
            }

            /**
             * @brief Führt einen Vorwärtspass durch das REN durch.
             * Abbruchkriterium ist entweder die maximale Anzahl an Iterationen oder eine Konvergenz unterhalb der Toleranz.
             * 
             * @param h_0 Der initiale Zustand.
             * @param fakeQuant Ob die Fake-Quantisierung verwendet werden soll.
             * @param maxIterations Die maximale Anzahl an Iterationen, bevor abgebrochen wird.
             * @param tol Die Toleranz für die Konvergenz.
             * @param alpha Der Gewichtungsfaktor für die Krasnoselskii-Mann Iteration.
             * @param stepSizeBacktracking Verringert die Schrittweite alpha wenn die Residuen größer werden, um die Konvergenz zu stabilisieren.
             */
            ForwardResult forward(const ML::Vector& h_0, bool fakeQuant,
                size_t maxIterations = std::numeric_limits<size_t>::max(), float tol = 1e-4f,
                float alpha = 0.5f, bool stepSizeBacktracking = true) const;

            /**
             * @brief Führt einen Rückwärtspass durch das REN durch und berechnet die Gradienten für die Master-Parameter.
             * 
             * @param forwardResult Das Ergebnis des Vorwärtspasses.
             * @param inputGrad Der Gradient nach der Fehlerfunktion (dL/d(output)).
             * @param fakeQuant Ob die Fake-Quantisierung verwendet werden soll.
             * @param tol Die Toleranz für den Löser des linearen Systems.
             */
            Gradients backward(const ForwardResult& forwardResult, const ML::Vector& inputGrad, bool fakeQuant, float tol = 1e-4f) const;
    
        private:

            /**
             * @brief Führt einen Schritt des Vorwärtspasses durch das REN durch und berechnet die neuen Voraktivierungen.
             * 
             * @param h_t Der aktuelle Zustand.
             * @param h_0 Der initiale Zustand.
             * @param q Die Quantisierungsstrategie.
             */
            template <typename Q>
            ML::Vector forwardImpl(const ML::Vector& h_t, const ML::Vector& h_0, Q q) const;

            /**
             * @brief Führt einen Rückwärtspass durch das REN durch und berechnet die Gradienten für die Master-Parameter.
             * 
             * @param h_opt Der konvergierte Zustand nach dem Vorwärtspass.
             * @param z_opt Der voraktivierte, konvergierte Zustand.
             * @param inputGrad Der Gradient nach der Fehlerfunktion (dL/d(output)).
             * @param tol Die Toleranz für den Löser des linearen Systems.
             * @param q Die Quantisierungsstrategie.
             */
            template <typename Q>
            Gradients backwardImpl(const ML::Vector& h_opt, const ML::Vector& z_opt, const ML::Vector& inputGrad, float tol, Q q) const;
    };

}

#endif