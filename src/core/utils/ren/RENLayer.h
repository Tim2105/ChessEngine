#ifndef REN_REN_LAYER_H
#define REN_REN_LAYER_H

#include <stdexcept>
#include <stddef.h>

#include "core/utils/ren/Math.h"

namespace REN {
    /**
     * @brief Ein Resonant Equilibrium Network (REN) ist ein neuronales Netzwerk, dass über mehrere Schritte zum
     * Ergebnis konvergiert. Das erlaubt eine dynamische Anpassung der Rechenzeit, abhängig von der Komplexität der Stellung.
     * Dabei besteht eine theoretische Garantie, dass das System immer zu einem einzigartigen Fixpunkt konvergiert.
     */
    class RENLayer {
        public:
            struct SurrogateWeights {
                REN::Matrix q;
                REN::Vector gammaRaw;
                size_t size;

                inline SurrogateWeights(size_t s) : q(s), gammaRaw(s), size(s) {}
            };

            struct Gradients {
                REN::Matrix q;
                REN::Vector gammaRaw;
                REN::Vector bias;
                REN::Vector outputGrad;
                size_t size;

                inline Gradients(size_t s) : q(s), gammaRaw(s), bias(s), outputGrad(s), size(s) {}
            };

        private:
            SurrogateWeights surrogateWeights;
            REN::Matrix transform;
            REN::Vector bias;
            size_t size;
            float epsilon;

            /**
             * @brief Konstruiert W_r anhand von W_q und gamma_raw.
             */
            void constructTransform();

            /**
             * @brief Führt einen Schritt des Vorwärtspasses durch das REN durch und berechnet die neuen Aktivierungen.
             * 
             * @param h_t Der aktuelle Zustand.
             * @param h_0 Der initiale Zustand.
             * @param alpha Der Gewichtungsfaktor für die Krasnoselskii-Mann Iteration.
             * @param q Die Quantisierungsstrategie.
             */
            template <typename Q>
            std::tuple<Vector, float> forwardImpl(const Vector& h_t, const Vector& h_0, float alpha, Q q) const;

            /**
             * @brief Führt einen Rückwärtspass durch das REN durch und berechnet die Gradienten für die Master-Parameter.
             * 
             * @param h_opt Der konvergierte Zustand nach dem Vorwärtspass.
             * @param z_opt Der voraktivierte, konvergierte Zustand.
             * @param h_0 Der initiale Zustand.
             * @param inputGrad Der Gradient nach der Fehlerfunktion (dL/d(output)).
             * @param tol Die Toleranz für den Löser des linearen Systems.
             * @param q Die Quantisierungsstrategie.
             */
            template <typename Q>
            Gradients backwardImpl(const Vector& h_opt, const Vector& z_opt, const Vector& h_0, const Vector& inputGrad, float tol, Q q) const;

        public:
            inline RENLayer(size_t size, float eps = 0.01) : surrogateWeights(size), transform(size), bias(size), size(size), epsilon(eps) {
                constructTransform();
            }

            inline RENLayer(const SurrogateWeights& mw, const Matrix& rt, const Vector& b, float eps = 0.01) :
                surrogateWeights(mw), transform(rt), bias(b), size(mw.size), epsilon(eps) {

                // Konsistenzprüfung
                if(rt.rows != size || rt.cols != size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and transform matrix.");

                if(b.size != size)
                    throw std::invalid_argument("Dimension mismatch between MasterWeights and bias vector.");
            }

            inline RENLayer(const SurrogateWeights& mw, float eps = 0.01) :
                surrogateWeights(mw), transform(mw.size), bias(mw.size), size(mw.size), epsilon(eps) {

                constructTransform();
            }

            void testSolver() const;
    };

}

#endif