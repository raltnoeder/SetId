#ifndef EXCEPTIONS_H
#define	EXCEPTIONS_H

class SyntaxException : public std::exception
{
  public:
    SyntaxException()
    {
    }
    virtual ~SyntaxException() noexcept
    {
    }
    SyntaxException(const SyntaxException& orig) = default;
    SyntaxException& operator=(const SyntaxException& orig) = default;
    SyntaxException(SyntaxException&& orig) = default;
    SyntaxException& operator=(SyntaxException&& orig) = default;
};

class AppException : public std::exception
{
  public:
    AppException()
    {
    }
    virtual ~AppException() noexcept
    {
    }
    AppException(const AppException& orig) = default;
    AppException& operator=(const AppException& orig) = default;
    AppException(AppException&& orig) = default;
    AppException& operator=(AppException&& orig) = default;
};

#endif // EXCEPTIONS_H
