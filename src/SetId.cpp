extern "C"
{
    #include <unistd.h>
    #include <sys/types.h>
    #include <errno.h>
    #include <grp.h>
}

#include <cstdio>
#include <stdexcept>
#include <new>
#include <memory>
#include <string>
#include <dsaext.h>
#include <integerparse.h>

#include <GroupIdList.h>
#include <exceptions.h>

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
        std::fprintf(
            stderr,
            "\x1b[0;38;5;196mError: Unparsable value \"%s\" specified for parameter \"%s\"\x1b[0m\n\n,",
            text.c_str(), param
        );
        throw SyntaxException();
    }
    return value;
}

// @throws AppException
static uid_t to_userid(const int64_t value)
{
    const uid_t uid_value = static_cast<uid_t> (value);
    const int64_t check_value = static_cast<int64_t> (uid_value);

    if (uid_value != check_value)
    {
        std::string msg("User id number ");
        msg += std::to_string(value);
        msg += " ouf of range for system data type uid_t";
        std::fputs(
            msg.c_str(),
            stderr
        );
        throw AppException();
    }

    return uid_value;
}

// @throws AppException
static gid_t to_groupid(const int64_t value)
{
    const gid_t gid_value = static_cast<uid_t> (value);
    const int64_t check_value = static_cast<int64_t> (gid_value);

    if (gid_value != check_value)
    {
        std::string msg("Group id number ");
        msg += std::to_string(value);
        msg += " ouf of range for system data type gid_t";
        std::fputs(
            msg.c_str(),
            stderr
        );
        throw AppException();
    }

    return gid_value;
}

// @throws SyntaxException
static void throw_missing_value(const char* param)
{
    std::fprintf(
        stderr,
        "\x1b[0;38;5;196mError: Missing value for parameter \"%s\"\x1b[0m\n\n",
        param
    );
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
                    std::fputs(
                        "\x1b[0;38;5;196mError: Multiple \"userid\" parameters\x1b[0m\n\n",
                        stderr
                    );
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
                    std::fputs(
                        "\x1b[0;38;5;196mError: Multiple \"groupid\" parameters\x1b[0m\n\n",
                        stderr
                    );
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
                    std::fputs(
                        "\x1b[0;38;5;196mError: Multiple \"groups\" parameters\x1b[0m\n\n",
                        stderr
                    );
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
                        std::fputs(
                            "\x1b[0;38;5;196mError: The number of supplemental groups that were specified exceeds\n"
                            "       the maximum number of groups supported by the setgroups() system call\x1b[0m\n\n",
                            stderr
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
                    std::fputs(
                        "\x1b[0;38;5;196mError: Unexpected value for parameter \"command\"\x1b[0m\n\n",
                        stderr
                    );
                    throw SyntaxException();
                }
                cmd_arg_idx = arg_idx + 1;
                break;
            }
            else
            {
                std::fprintf(
                    stderr,
                    "\x1b[0;38;5;196mError: Invalid parameter \"%s\"\x1b[0mn\n",
                    key.c_str()
                );
                throw SyntaxException();
            }
        }

        if (cmd_arg_idx >= static_cast<size_t> (argc))
        {
            std::fputs(
                "\x1b[0;38;5;196mError: Missing command (executable_path)\x1b[0m\n\n",
                stderr
            );
            throw SyntaxException();
        }

        if (have_groupid)
        {
            if (setregid(req_groupid, req_groupid) != 0)
            {
                std::fputs(
                    "\x1b[0;38;5;196mError: Adjusting the group id failed\x1b[0m\n\n",
                    stderr
                );
                throw AppException();
            }
        }

        if (have_groups)
        {
            if (setgroups(static_cast<int> (sup_groups_size), sup_groups.get()) != 0)
            {
                std::fputs(
                    "\x1b[0;38;5;196mError: Adjusting the list of supplemental groups failed\x1b[0m\n\n",
                    stderr
                );
                throw AppException();
            }
        }
        else
        if (setgroups(0, nullptr) != 0)
        {
            std::fputs(
                "\x1b[0;38;5;196mError: Clearing the list of supplemental groups failed\x1b[0m\n\n",
                stderr
            );
            throw AppException();
        }

        if (have_userid)
        {
            if (setreuid(req_userid, req_userid) != 0)
            {
                std::fputs(
                    "\x1b[0;38;5;196mError: Adjusting the user id failed\x1b[0m\n\n",
                    stderr
                );
                throw AppException();
            }
        }

        execve(argv[cmd_arg_idx], &argv[cmd_arg_idx], environ);

        fprintf(
            stderr,
            "\x1b[0;38;5;196mError: Executing the command failed, executable_path = %s\x1b[0m\n\n",
            argv[cmd_arg_idx]
        );
    }
    catch (SyntaxException&)
    {
        std::fprintf(
            stderr,
            "Syntax: %s [ userid=<uid_nr> ] [ groupid=<gid_nr> ] [ groups=<gid_nr>,<gid_nr>,... ] "
            "command <executable_path> <arguments...>\n",
            exec_name
        );
    }
    catch (AppException&)
    {
        // Error reported by throwing function
    }
    catch (std::bad_alloc&)
    {
        std::fputs(
            "\x1b[0;38;5;196mError: Out of memory\x1b[0m\n\n",
            stderr
        );
    }
    std::fflush(stdout);
    std::fflush(stderr);

    return rc;
}
