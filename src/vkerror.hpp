#pragma once

#include "vkdispatch.hpp"

namespace layer
{

    class VkError
    {

    public:

        VkError(VkResult result)
        : m_result(result) { }

        VkResult result() const
        {
            return m_result;
        }

    private:

        VkResult m_result;

    };

}