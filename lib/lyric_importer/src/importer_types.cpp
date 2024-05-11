//
//#include <lyric_importer/importer_types.h>
//#include "lyric_importer/importer_result.h"
//
//lyric_importer::ImportedAssembly::ImportedAssembly(
//    const lyric_common::AssemblyLocation &location,
//    ImportMode mode,
//    lyric_object::LyricObject object,
//    int numSymbols)
//    : m_location(location),
//      m_mode(mode),
//      m_object(object),
//      m_incompleteSymbols(numSymbols)
//{
//    TU_ASSERT (m_location.isValid());
//    TU_ASSERT (m_object.isValid());
//    m_incompleteSymbols.addRange(0, numSymbols);
//}
//
//lyric_common::AssemblyLocation
//lyric_importer::ImportedAssembly::getLocation() const
//{
//    return m_location;
//}
//
//lyric_importer::ImportMode
//lyric_importer::ImportedAssembly::getImportMode() const
//{
//    return m_mode;
//}
//
//lyric_object::LyricObject
//lyric_importer::ImportedAssembly::getAssembly() const
//{
//    return m_object;
//}
//
//bool
//lyric_importer::ImportedAssembly::hasSymbol(const lyric_common::SymbolPath &path) const
//{
//    return m_importedSymbols.contains(path);
//}
//
//tempo_utils::Status
//lyric_importer::ImportedAssembly::insertSymbol(const lyric_common::SymbolPath &path)
//{
//    if (m_importedSymbols.contains(path))
//        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
//            "duplicate symbol {} in assembly {}", path.toString(), m_location.toString());
//    m_importedSymbols.insert(path);
//    return ImporterStatus::ok();
//}
//
//absl::flat_hash_set<lyric_common::SymbolPath>::const_iterator
//lyric_importer::ImportedAssembly::symbolsBegin() const
//{
//    return m_importedSymbols.cbegin();
//}
//
//absl::flat_hash_set<lyric_common::SymbolPath>::const_iterator
//lyric_importer::ImportedAssembly::symbolsEnd() const
//{
//    return m_importedSymbols.cend();
//}
//
//bool
//lyric_importer::ImportedAssembly::hasTemplate(tu_uint32 templateIndex) const
//{
//    return templateIndex < m_importedTemplates.size();
//}
//
//lyric_common::SymbolUrl
//lyric_importer::ImportedAssembly::getTemplate(tu_uint32 templateIndex) const
//{
//    if (templateIndex < m_importedTemplates.size())
//        return m_importedTemplates.at(templateIndex);
//    return {};
//}
//
//tempo_utils::Status
//lyric_importer::ImportedAssembly::insertTemplate(
//    tu_uint32 templateIndex,
//    const lyric_common::SymbolUrl &templateUrl)
//{
//    if (templateIndex != m_importedTemplates.size())
//        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
//            "expected template index {} but encountered {}", m_importedTemplates.size(), templateIndex);
//    m_importedTemplates.push_back(templateUrl);
//    return ImporterStatus::ok();
//}
//
//bool
//lyric_importer::ImportedAssembly::hasType(tu_uint32 typeIndex) const
//{
//    return typeIndex < m_importedTypes.size();
//}
//
//lyric_assembler::Assignable
//lyric_importer::ImportedAssembly::getType(tu_uint32 typeIndex) const
//{
//    if (typeIndex < m_importedTypes.size())
//        return m_importedTypes.at(typeIndex);
//    return {};
//}
//
//tempo_utils::Status
//lyric_importer::ImportedAssembly::insertType(
//    tu_uint32 typeIndex,
//    const lyric_assembler::Assignable &type)
//{
//    if (typeIndex != m_importedTypes.size())
//        return ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
//            "expected type index {} but encountered {}", m_importedTypes.size(), typeIndex);
//    m_importedTypes.push_back(type);
//    return ImporterStatus::ok();
//}
//
//bool
//lyric_importer::ImportedAssembly::isComplete(tu_uint32 symbolIndex) const
//{
//    return !m_incompleteSymbols.contains(symbolIndex);
//}
//
//void
//lyric_importer::ImportedAssembly::setComplete(tu_uint32 symbolIndex)
//{
//    m_incompleteSymbols.remove(symbolIndex);
//}
//
//void
//lyric_importer::ImportedAssembly::clearComplete(tu_uint32 symbolIndex)
//{
//    m_incompleteSymbols.add(symbolIndex);
//}
//
//tu_uint32
//lyric_importer::ImportedAssembly::numRemaining() const
//{
//    return m_incompleteSymbols.count();
//}
//
//bool
//lyric_importer::ImportedAssembly::allCompleted() const
//{
//    return m_incompleteSymbols.isEmpty();
//}
//
//lyric_importer::ImportCache::ImportCache()
//{
//}
//
//bool
//lyric_importer::ImportCache::hasImportedAssembly(const lyric_common::AssemblyLocation &location)
//{
//    return m_importedAssemblies.contains(location);
//}
//
//std::shared_ptr<lyric_importer::ImportedAssembly>
//lyric_importer::ImportCache::getImportedAssembly(const lyric_common::AssemblyLocation &location)
//{
//    if (m_importedAssemblies.contains(location))
//        return m_importedAssemblies.at(location);
//    return {};
//}
//
//std::shared_ptr<lyric_importer::ImportedAssembly>
//lyric_importer::ImportCache::insertImportedAssembly(
//    const lyric_common::AssemblyLocation &location,
//    ImportMode mode,
//    lyric_object::LyricObject object)
//{
//    if (m_importedAssemblies.contains(location))
//        return {};
//
//    TU_ASSERT (object.isValid());
//    auto walker = object.getObject();
//    TU_ASSERT (walker.isValid());
//    auto numSymbols= walker.numSymbols();
//
//    auto importedAssembly = std::make_shared<ImportedAssembly>(location, mode, object, numSymbols);
//    m_importedAssemblies[location] = importedAssembly;
//    return importedAssembly;
//}