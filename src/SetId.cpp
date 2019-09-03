extern "C"
{
    #include <unistd.h>
    #include <sys/types.h>
    #include <errno.h>
    #include <grp.h>
}

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <new>
#include <memory>
#include <string>
#include <dsaext.h>
#include <integerparse.h>

#include <GroupIdList.h>
#include <exceptions.h>

extern char** environ;

static const std::string FMT_ALERT("\x1B[0;38;5;196m");
static const std::string FMT_RESET("\x1B[0m");


template<typename T>
static void error_msg(const T msg)
{
    std::cerr << FMT_ALERT << msg << FMT_RESET << "\n\n" << std::flush;
}

// @throws SyntaxException
static int64_t to_numeric(const char* param, const std::string& text)
{
    int64_t value;
    try
    {
        value = dsaext::parse_signed_int64(text);
    }
    catch (dsaext::NumberFormatException&)
    {
        std::cerr << FMT_ALERT << "Error: Unparsable value \"" << text << "\" specified for parameter \"" <<
            param << FMT_RESET << "\n\n" << std::flush;
        throw SyntaxException();
    }
    return value;
}

// @throws std::bad_alloc, AppException
static uid_t to_userid(const int64_t value)
{
    const uid_t uid_value = static_cast<uid_t> (value);
    const int64_t check_value = static_cast<int64_t> (uid_value);

    if (uid_value != check_value)
    {
        std::string msg("User id number ");
        msg += std::to_string(value);
        msg += " ouf of range for system data type uid_t";
        error_msg(msg);
        throw AppException();
    }

    return uid_value;
}

// @throws std::bad_alloc, AppException
static gid_t to_groupid(const int64_t value)
{
    const gid_t gid_value = static_cast<uid_t> (value);
    const int64_t check_value = static_cast<int64_t> (gid_value);

    if (gid_value != check_value)
    {
        std::string msg("Group id number ");
        msg += std::to_string(value);
        msg += " ouf of range for system data type gid_t";
        error_msg(msg);
        throw AppException();
    }

    return gid_value;
}

// @throws SyntaxException
static void throw_missing_value(const char* param)
{
    std::cerr << FMT_ALERT << "Error: Missing value for parameter \"" << param << "\"" << FMT_RESET << "\n\n" <<
        std::flush;
    throw SyntaxException();
}

int main(int argc, char* argv[])
{
    int rc = EXIT_FAILURE;
    const char* exec_name = "setid";
    try
    {
        if (argc < 3)
        {
            throw SyntaxException();
        }

        bool have_userid = false;
        bool have_groupid = false;
        bool have_groups = false;

        uid_t req_userid;
        gid_t req_groupid;
        std::unique_ptr<gid_t[]> sup_groups;
        size_t sup_groups_size;

        size_t cmd_arg_idx = 1;
        for (size_t arg_idx = 1; arg_idx < static_cast<size_t> (argc); ++arg_idx)
        {
            std::string cur_arg(argv[arg_idx]);
            std::string key;
            std::string value;

            bool have_value = false;
            {
                size_t split_idx = cur_arg.find("=");
                if (split_idx != std::string::npos)
                {
                    have_value = true;
                    key = cur_arg.substr(0, split_idx);
                    value = cur_arg.substr(split_idx + 1);
                }
                else
                {
                    key = cur_arg;
                }
            }

            if (key == "userid")
            {
                if (have_userid)
                {
                    error_msg("Error: Multiple \"userid\" parameters");
                    throw SyntaxException();
                }

                if (!have_value)
                {
                    throw_missing_value("userid");
                }
                req_userid = to_userid(to_numeric("userid", value));
                have_userid = true;
            }
            else
            if (key == "groupid")
            {
                if (have_groupid)
                {
                    error_msg("Error: Multiple \"groupid\" parameters");
                    throw SyntaxException();
                }
                req_groupid = to_groupid(to_numeric("groupid", value));
                have_groupid = true;
            }
            else
            if (key == "groups")
            {
                if (have_groups)
                {
                    error_msg("Error: Multiple \"groups\" parameters");
                    throw SyntaxException();
                }

                if (!have_value)
                {
                    throw_missing_value("groups");
                }

                GroupIdList group_list;

                int counter = 0;
                while (value.length() > 0)
                {
                    size_t split_idx = value.find(",");
                    std::string group_id_str;
                    if (split_idx != std::string::npos)
                    {
                        group_id_str = value.substr(0, split_idx);
                        value = value.substr(split_idx + 1);
                    }
                    else
                    {
                        group_id_str = value;
                        value = "";
                    }

                    if (counter < INT_MAX)
                    {
                        group_list.add_entry(to_groupid(to_numeric("groups", group_id_str)));
                        ++counter;
                    }
                    else
                    {
                        error_msg(
                            "Error: The number of supplemental groups that were specified exceeds\n"
                            "       the maximum number of groups supported by the setgroups() system call"
                        );
                        throw AppException();
                    }
                }

                sup_groups = std::unique_ptr<gid_t[]>(group_list.get_array());
                sup_groups_size = group_list.get_size();
                have_groups = true;
            }
            else
            if (key == "command")
            {
                if (have_value)
                {
                    error_msg("Error: Unexpected value for parameter \"command\"");
                    throw SyntaxException();
                }
                cmd_arg_idx = arg_idx + 1;
                break;
            }
            else
            {
                std::cerr << FMT_ALERT << "Error: Invalid parameter \"" << key << "\"" << FMT_RESET <<
                    "\n\n" << std::flush;
                throw SyntaxException();
            }
        }

        if (cmd_arg_idx >= static_cast<size_t> (argc))
        {
            error_msg("Error: Missing command (executable_path)");
            throw SyntaxException();
        }

        if (have_groupid)
        {
            if (setregid(req_groupid, req_groupid) != 0)
            {
                error_msg("Error: Adjusting the group id failed");
                throw AppException();
            }
        }

        if (have_groups)
        {
            if (setgroups(static_cast<int> (sup_groups_size), sup_groups.get()) != 0)
            {
                error_msg("Error: Adjusting the list of supplemental groups failed");
                throw AppException();
            }
        }
        else
        if (setgroups(0, nullptr) != 0)
        {
            error_msg("Error: Clearing the list of supplemental groups failed");
            throw AppException();
        }

        if (have_userid)
        {
            if (setreuid(req_userid, req_userid) != 0)
            {
                error_msg("Error: Adjusting the user id failed");
                throw AppException();
            }
        }

        execve(argv[cmd_arg_idx], &argv[cmd_arg_idx], environ);

        std::cerr << FMT_ALERT << "Error: Executing the command failed, executable_path = " <<
            argv[cmd_arg_idx] << FMT_RESET << "\n\n" << std::flush;
    }
    catch (SyntaxException&)
    {
        std::cerr << "Syntax: " << exec_name <<
            " [ userid=<uid_nr> ] [ groupid=<gid_nr> ] [ groups=<gid_nr>,<gid_nr>,... ] "
            "command <executable_path> <arguments...>\n\n" << std::flush;
    }
    catch (AppException&)
    {
        // Error reported by throwing function
    }
    catch (std::bad_alloc&)
    {
        error_msg("Error: Out of memory");
    }

    std::cerr << std::flush;
    std::cout << std::flush;

    return rc;
}
