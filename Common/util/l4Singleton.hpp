#pragma once

template <typename T>
class l4Singleton
{
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
};