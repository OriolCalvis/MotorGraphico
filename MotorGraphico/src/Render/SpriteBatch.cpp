#include "Render/SpriteBatch.h"

#include "Core/Resources/Texture.h"

#include <cstddef>

#include <glad/glad.h>

SpriteBatch::SpriteBatch() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, pos)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

SpriteBatch::~SpriteBatch() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void SpriteBatch::begin() {
    m_vertices.clear();
    m_currentTexture = nullptr;
}

void SpriteBatch::submit(const Vector2& pos, const Vector2& size, const UVRect& uv, Texture* tex) {
    if (tex != m_currentTexture && m_currentTexture != nullptr) {
        flush();
    }
    m_currentTexture = tex;

    Vector2 topLeft = pos;
    Vector2 topRight{pos.x + size.x, pos.y};
    Vector2 bottomLeft{pos.x, pos.y + size.y};
    Vector2 bottomRight{pos.x + size.x, pos.y + size.y};

    // Dos triangulos (TL,TR,BL) y (TR,BR,BL) comparten la diagonal TR-BL.
    m_vertices.push_back({topLeft, {uv.u0, uv.v0}});
    m_vertices.push_back({topRight, {uv.u1, uv.v0}});
    m_vertices.push_back({bottomLeft, {uv.u0, uv.v1}});

    m_vertices.push_back({topRight, {uv.u1, uv.v0}});
    m_vertices.push_back({bottomRight, {uv.u1, uv.v1}});
    m_vertices.push_back({bottomLeft, {uv.u0, uv.v1}});
}

Result<bool> SpriteBatch::flush() {
    if (m_vertices.empty()) {
        return Result<bool>::Ok(false);
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long>(m_vertices.size() * sizeof(Vertex)),
                 m_vertices.data(), GL_DYNAMIC_DRAW);

    if (m_currentTexture != nullptr) {
        m_currentTexture->bind(0);
    }

    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_vertices.size()));

    m_vertices.clear();
    return Result<bool>::Ok(true);
}

void SpriteBatch::end() { flush(); }
