#ifndef REN_REN_LAYER_H
#define REN_REN_LAYER_H

#include <stdexcept>
#include <stddef.h>

#include "tune/ml/Math.h"

namespace REN {
    /**
     * @brief Ein Resonant Equilibrium Network (REN) ist ein neuronales Netzwerk, dass über mehrere Schritte zum
     * Ergebnis konvergiert. Das erlaubt eine dynamische Anpassung der Rechenzeit, abhängig von der Komplexität der Stellung.
     * Dabei besteht eine theoretische Garantie, dass das System immer zu einem einzigartigen Fixpunkt konvergiert.
     */
    class RENLayer {
        public:
            struct SurrogateWeights {
                ML::Matrix q;
                ML::Vector gammaRaw;
                size_t size;

                inline SurrogateWeights(size_t s) : q(s), gammaRaw(s), size(s) {}
            };

            struct Gradients {
                ML::Matrix q;
                ML::Vector gammaRaw;
                ML::Vector bias;
                ML::Vector inputGrad;
                size_t size;

                inline Gradients(size_t s) : q(s), gammaRaw(s), bias(s), inputGrad(s), size(s) {}
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
            ML::Matrix transform;
            ML::Vector bias;
            size_t size;
            float epsilon;

            /**
             * @brief Konstruiert W_r anhand von W_q und gamma_raw.
             */
            void constructTransform();

            inline RENLayer(size_t size, float eps = 0.01) :
                surrogateWeights(size), transform(size), bias(size), size(size), epsilon(eps) {
                
                constructTransform();
            }

            inline RENLayer(const SurrogateWeights& mw, const ML::Matrix& rt, const ML::Vector& b, float eps = 0.01) :
                surrogateWeights(mw), transform(rt), bias(b), size(mw.size), epsilon(eps) {

                // Konsistenzprüfung
                if(rt.innerDim != size || rt.outerDim != size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and transform matrix.");

                if(b.size != size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and bias vector.");
            }

            inline RENLayer(const SurrogateWeights& mw, float eps = 0.01) :
                surrogateWeights(mw), transform(mw.size), bias(mw.size), size(mw.size), epsilon(eps) {

                constructTransform();
            }

            /**
             * @brief Führt einen Vorwärtspass durch das REN durch.
             * Abbruchkriterium ist entweder die maximale Anzahl an Iterationen oder eine Konvergenz unterhalb der Toleranz.
             * 
             * @param h_0 Der initiale Zustand.
             * @param fakeQuant Ob die Fake-Quantisierung verwendet werden soll.
             * @param alpha Der Gewichtungsfaktor für die Krasnoselskii-Mann Iteration.
             * @param maxIterations Die maximale Anzahl an Iterationen, bevor abgebrochen wird.
             * @param tol Die Toleranz für die Konvergenz.
             */
            ForwardResult forward(const ML::Vector& h_0, bool fakeQuant, float alpha = 0.5f,
                size_t maxIterations = std::numeric_limits<size_t>::max(), float tol = 1e-4f) const;

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