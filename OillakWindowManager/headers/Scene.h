#pragma once
#include "Model.h"
#include <vector>
#include <memory>

class Scene {
public:
    void addModel(std::unique_ptr<Model> model) {
        m_models.push_back(std::move(model));
    }


    const std::vector<std::unique_ptr<Model>>& getModels() const { return m_models; }

private:
    std::vector<std::unique_ptr<Model>> m_models;
};