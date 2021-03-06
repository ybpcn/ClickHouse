#pragma once

#include <Common/NamePrompter.h>
#include <Parsers/IAST_fwd.h>
#include <Storages/ColumnsDescription.h>
#include <Storages/ConstraintsDescription.h>
#include <Storages/IStorage_fwd.h>
#include <Storages/registerStorages.h>
#include <unordered_map>


namespace DB
{

class Context;
class ASTCreateQuery;
class ASTStorage;


/** Allows to create a table by the name and parameters of the engine.
  * In 'columns' Nested data structures must be flattened.
  * You should subsequently call IStorage::startup method to work with table.
  */
class StorageFactory : private boost::noncopyable, public IHints<1, StorageFactory>
{
public:

    static StorageFactory & instance();

    struct Arguments
    {
        const String & engine_name;
        ASTs & engine_args;
        ASTStorage * storage_def;
        const ASTCreateQuery & query;
        const String & data_path;
        const String & table_name;
        const String & database_name;
        Context & local_context;
        Context & context;
        const ColumnsDescription & columns;
        const ConstraintsDescription & constraints;
        bool attach;
        bool has_force_restore_data_flag;
    };

    using Creator = std::function<StoragePtr(const Arguments & arguments)>;

    StoragePtr get(
        const ASTCreateQuery & query,
        const String & data_path,
        const String & table_name,
        const String & database_name,
        Context & local_context,
        Context & context,
        const ColumnsDescription & columns,
        const ConstraintsDescription & constraints,
        bool attach,
        bool has_force_restore_data_flag) const;

    /// Register a table engine by its name.
    /// No locking, you must register all engines before usage of get.
    void registerStorage(const std::string & name, Creator creator);

    const auto & getAllStorages() const
    {
        return storages;
    }

    std::vector<String> getAllRegisteredNames() const override
    {
        std::vector<String> result;
        auto getter = [](const auto & pair) { return pair.first; };
        std::transform(storages.begin(), storages.end(), std::back_inserter(result), getter);
        return result;
    }

private:
    using Storages = std::unordered_map<std::string, Creator>;
    Storages storages;
};

}
