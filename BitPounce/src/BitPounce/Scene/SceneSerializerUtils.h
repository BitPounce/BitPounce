#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann
{
    template<int L, typename T, glm::qualifier Q>
    struct adl_serializer<glm::vec<L, T, Q>>
    {
        static void to_json(json& j, const glm::vec<L, T, Q>& v)
        {
            j = json::array();

            for (int i = 0; i < L; i++)
                j.push_back(v[i]);
        }

        static void from_json(const json& j, glm::vec<L, T, Q>& v)
        {
            if (j.is_array())
            {
                for (int i = 0; i < L; i++)
                    v[i] = j.at(i).get<T>();
            }
            else
            {
                static const char* names[] = { "x","y","z","w" };

                for (int i = 0; i < L; i++)
                    v[i] = j.at(names[i]).get<T>();
            }
        }
    };

    template<int C, int R, typename T, glm::qualifier Q>
    struct adl_serializer<glm::mat<C, R, T, Q>>
    {
        static void to_json(json& j, const glm::mat<C, R, T, Q>& m)
        {
            j = json::array();

            for (int c = 0; c < C; c++)
            {
                json column = json::array();

                for (int r = 0; r < R; r++)
                    column.push_back(m[c][r]);

                j.push_back(column);
            }
        }

        static void from_json(const json& j, glm::mat<C, R, T, Q>& m)
        {
            for (int c = 0; c < C; c++)
            {
                for (int r = 0; r < R; r++)
                    m[c][r] = j.at(c).at(r).get<T>();
            }
        }
    };
}