//----------------------------------------------------------------------------------------------------------------------
// XmlPackageLoader.cpp
//
// Copyright (C) 2010 WhiteMoon Dreams, Inc.
// All Rights Reserved
//----------------------------------------------------------------------------------------------------------------------

#include "EnginePch.h"
#include "Engine/ArchivePackageLoader.h"

#include "Foundation/File/File.h"
#include "Foundation/File/Path.h"
#include "Foundation/File/Directory.h"
#include "Foundation/Stream/FileStream.h"
#include "Foundation/Stream/BufferedStream.h"
#include "Foundation/AsyncLoader.h"
//#include "Engine/BinaryDeserializer.h"
#include "Engine/CacheManager.h"
#include "Engine/Config.h"
//#include "Engine/DirectDeserializer.h"
//#include "Engine/DirectSerializer.h"
#include "Engine/NullLinker.h"
#include "Engine/GameObjectLoader.h"
#include "Engine/Resource.h"
//#include "PcSupport/ObjectPreprocessor.h"
//#include "PcSupport/ResourceHandler.h"
#include "Foundation/Reflect/ArchiveXML.h"
#include "Foundation/Reflect/Data/DataDeduction.h"

#include "expat.h"

using namespace Helium;

REFLECT_DEFINE_OBJECT(ObjectDescriptor);

void Helium::ObjectDescriptor::PopulateComposite( Reflect::Composite& comp )
{
    comp.AddField(&ObjectDescriptor::m_Name, TXT("m_Name"));
    comp.AddField(&ObjectDescriptor::m_TypeName, TXT("m_TypeName"));
    comp.AddField(&ObjectDescriptor::m_OwnerPath, TXT("m_OwnerPath"));
    comp.AddField(&ObjectDescriptor::m_TemplatePath, TXT("m_TemplatePath"));
}


/// Constructor.
ArchivePackageLoader::ArchivePackageLoader()
    : m_startPreloadCounter( 0 )
    , m_preloadedCounter( 0 )
    , m_loadRequestPool( LOAD_REQUEST_POOL_BLOCK_SIZE )
    , m_parentPackageLoadId( Invalid< size_t >() )
{
}

/// Destructor.
ArchivePackageLoader::~ArchivePackageLoader()
{
    Shutdown();
}

/// Initialize this package loader.
///
/// @param[in] packagePath  GameObject path of the package to load.
///
/// @return  True if this loader was initialized successfully, false if not.
///
/// @see Shutdown()
bool ArchivePackageLoader::Initialize( GameObjectPath packagePath )
{
    Shutdown();

    // Make sure the path represents a package.
    if( packagePath.IsEmpty() )
    {
        HELIUM_TRACE( TRACE_ERROR, TXT( "ArchivePackageLoader::Initialize(): Empty package path specified.\n" ) );

        return false;
    }

    HELIUM_TRACE(
        TRACE_DEBUG,
        TXT( "ArchivePackageLoader::Initialize(): Initializing loader for package \"%s\".\n" ),
        *packagePath.ToString() );

    if( !packagePath.IsPackage() )
    {
        HELIUM_TRACE(
            TRACE_ERROR,
            TXT( "ArchivePackageLoader::Initialize(): \"%s\" does not represent a package path.\n" ),
            *packagePath.ToString() );

        return false;
    }

    // Store the package path.
    m_packagePath = packagePath;

    // Attempt to locate the specified package if it already happens to exist.
    m_spPackage = GameObject::Find< Package >( packagePath );
    Package* pPackage = m_spPackage;
    if( pPackage )
    {
        if( pPackage->GetLoader() )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "ArchivePackageLoader::Initialize(): Package \"%s\" already has a loader.\n" ),
                *packagePath.ToString() );

            m_spPackage.Release();

            return false;
        }

        pPackage->SetLoader( this );
    }
    else
    {
        // Make sure we don't have a name clash with a non-package object.
        GameObjectPtr spObject( GameObject::FindObject( packagePath ) );
        if( spObject )
        {
            HELIUM_ASSERT( !spObject->IsPackage() );

            HELIUM_TRACE(
                TRACE_ERROR,
                ( TXT( "PackageLoader::Initialize(): Package loader cannot be initialized for \"%s\", as an " )
                TXT( "object with the same name exists that is not a package.\n" ) ),
                *packagePath.ToString() );

            return false;
        }
    }

    // Build the package file path.  If the package is a user configuration package, use the user data directory,
    // otherwise use the global data directory.
    Config& rConfig = Config::GetStaticInstance();
    Path dataDirectory;

    if ( packagePath == rConfig.GetUserConfigPackagePath() )
    {
        if ( !File::GetUserDataDirectory( dataDirectory ) )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "PackageLoader::Initialize(): Could not obtain user data directory." ) );

            return false;
        }
    }
    else
    {
        if ( !File::GetDataDirectory( dataDirectory ) )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "PackageLoader::Initialize(): Could not obtain user data directory." ) );

            return false;
        }
    }

    m_packageFilePath = dataDirectory + packagePath.ToFilePathString().GetData() + TXT("/");
    if (!m_packageFilePath.IsDirectory())
    {
        return false;
    }
    
    // Retrieve the size of the package file.  Note that we still keep around the loader even if the package file
    // isn't found so as to allow for memory-only packages or the creation of new packages at editor runtime.
    //SetInvalid( m_packageFileSize );

    //int64_t packageFileSize = m_packageFilePath.Size();
    //if( packageFileSize == -1 )
    //{
    //    HELIUM_TRACE(
    //        TRACE_WARNING,
    //        TXT( "ArchivePackageLoader::Initialize(): Failed to locate a valid package file for package \"%s\".\n" ),
    //        *m_packageFilePath,
    //        *packagePath.ToString() );
    //}
    //else if( packageFileSize == 0 )
    //{
    //    HELIUM_TRACE(
    //        TRACE_WARNING,
    //        TXT( "ArchivePackageLoader::Initialize(): Package file \"%s\" for package \"%s\" is empty.\n" ),
    //        *m_packageFilePath,
    //        *packagePath.ToString() );
    //}
    //else if( static_cast< uint64_t >( packageFileSize ) > static_cast< uint64_t >( ~static_cast< size_t >( 0 ) ) )
    //{
    //    HELIUM_TRACE(
    //        TRACE_ERROR,
    //        ( TXT( "ArchivePackageLoader::Initialize(): Package file \"%s\" exceeds the maximum size supported by " )
    //        TXT( "the current platform (package: %" ) TPRIu64 TXT( " bytes; max supported: %" ) TPRIuSZ
    //        TXT( " bytes).\n" ) ),
    //        *packagePath.ToString(),
    //        static_cast< uint64_t >( packageFileSize ),
    //        ~static_cast< size_t >( 0 ) );

    //    if( pPackage )
    //    {
    //        pPackage->SetLoader( NULL );
    //    }

    //    m_spPackage.Release();

    //    return false;
    //}
    //else
    //{
    //    m_packageFileSize = static_cast< size_t >( packageFileSize );
    //}

    return true;
}

/// @copydoc PackageLoader::Shutdown()
void ArchivePackageLoader::Shutdown()
{
    // Sync with any in-flight async load requests.
    if( m_startPreloadCounter )
    {
        while( !TryFinishPreload() )
        {
            Tick();
        }
    }
    
    HELIUM_ASSERT( IsInvalid( m_parentPackageLoadId ) );

    // Unset the reference back to this loader in the package.
    Package* pPackage = m_spPackage;
    if( pPackage )
    {
        pPackage->SetLoader( NULL );
    }

    m_spPackage.Release();
    m_packagePath.Clear();

    AtomicExchangeRelease( m_startPreloadCounter, 0 );
    AtomicExchangeRelease( m_preloadedCounter, 0 );

    m_objects.Clear();

    size_t loadRequestCount = m_loadRequests.GetSize();
    for( size_t requestIndex = 0; requestIndex < loadRequestCount; ++requestIndex )
    {
        if( m_loadRequests.IsElementValid( requestIndex ) )
        {
            LoadRequest* pRequest = m_loadRequests[ requestIndex ];
            HELIUM_ASSERT( pRequest );
            m_loadRequestPool.Release( pRequest );
        }
    }

    m_loadRequests.Clear();

    m_packageFilePath.Clear();
}

/// Begin asynchronous pre-loading of package information.
///
/// @see TryFinishPreload()
bool ArchivePackageLoader::BeginPreload()
{
    MutexScopeLock scopeLock( m_accessLock );

    HELIUM_ASSERT( !m_startPreloadCounter );
    HELIUM_ASSERT( !m_preloadedCounter );
    HELIUM_ASSERT( IsInvalid( m_parentPackageLoadId ) );

    // Load the parent package if we need to create the current package.
    if( !m_spPackage )
    {
        GameObjectPath parentPackagePath = m_packagePath.GetParent();
        if( !parentPackagePath.IsEmpty() )
        {
            GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
            HELIUM_ASSERT( pObjectLoader );

            m_parentPackageLoadId = pObjectLoader->BeginLoadObject( parentPackagePath );
        }
    }

    //PMD: Changed to walk through all files and load them in
    AsyncLoader &rAsyncLoader = AsyncLoader::GetStaticInstance();
    
    Directory packageDirectory( m_packageFilePath );
    for( ; !packageDirectory.IsDone(); packageDirectory.Next() )
    {
        const DirectoryItem& item = packageDirectory.GetItem();
        if (item.m_Path.Extension() == TXT("object"))
        {
            FileReadRequest *request = m_fileReadRequests.New();
            request->expectedSize = item.m_Size;

            // Create a buffer for the file to be read into temporarily
            request->pLoadBuffer = DefaultAllocator().Allocate( item.m_Size );
            HELIUM_ASSERT( request->pLoadBuffer );

            // Queue up the read
            request->asyncLoadId = rAsyncLoader.QueueRequest( request->pLoadBuffer, String( item.m_Path.c_str() ), 0, item.m_Size );
            HELIUM_ASSERT( IsValid( request->asyncLoadId ) );
        }
    }

    AtomicExchangeRelease( m_startPreloadCounter, 1 );

    return true;
}

/// @copydoc PackageLoader::TryFinishPreload()
bool ArchivePackageLoader::TryFinishPreload()
{
    return ( m_preloadedCounter != 0 );
}

/// @copydoc PackageLoader::BeginLoadObject()
size_t ArchivePackageLoader::BeginLoadObject( GameObjectPath path )
{
    MutexScopeLock scopeLock( m_accessLock );

    // Make sure preloading has completed.
    HELIUM_ASSERT( m_preloadedCounter != 0 );
    if( !m_preloadedCounter )
    {
        return Invalid< size_t >();
    }

    // If this package is requested, simply provide the (already loaded) package instance.
    if( path == m_packagePath )
    {
        LoadRequest* pRequest = m_loadRequestPool.Allocate();
        HELIUM_ASSERT( pRequest );

        HELIUM_ASSERT( m_spPackage );
        pRequest->spObject = m_spPackage;

        SetInvalid( pRequest->index );
        HELIUM_ASSERT( pRequest->linkTable.IsEmpty() );
        HELIUM_ASSERT( !pRequest->spType );
        HELIUM_ASSERT( !pRequest->spTemplate );
        HELIUM_ASSERT( !pRequest->spOwner );
        SetInvalid( pRequest->templateLoadId );
        SetInvalid( pRequest->ownerLoadId );
        SetInvalid( pRequest->persistentResourceDataLoadId );
        pRequest->pCachedObjectDataBuffer = NULL;
        pRequest->cachedObjectDataBufferSize = 0;

        pRequest->flags = LOAD_FLAG_PRELOADED;

        size_t requestId = m_loadRequests.Add( pRequest );

        return requestId;
    }

    // Locate the object within this package.
    size_t objectCount = m_objects.GetSize();
    size_t objectIndex;
    for( objectIndex = 0; objectIndex < objectCount; ++objectIndex )
    {
        SerializedObjectData& rObjectData = m_objects[ objectIndex ];
        if( rObjectData.objectPath == path )
        {
            break;
        }
    }

    if( objectIndex >= objectCount )
    {
        HELIUM_TRACE(
            TRACE_WARNING,
            TXT( "ArchivePackageLoader::BeginLoadObject(): Failed to locate \"%s\" for loading.\n" ),
            *path.ToString() );

        return Invalid< size_t >();
    }

    SerializedObjectData& rObjectData = m_objects[ objectIndex ];

    // Locate the type object.
    HELIUM_ASSERT( !rObjectData.typeName.IsEmpty() );
    GameObjectType* pType = GameObjectType::Find( rObjectData.typeName );
    if( !pType )
    {
        HELIUM_TRACE(
            TRACE_ERROR,
            TXT( "ArchivePackageLoader::BeginLoadObject(): Failed to locate type \"%s\" for loading object \"%s\".\n" ),
            *rObjectData.typeName,
            *path.ToString() );

        return Invalid< size_t >();
    }

#ifndef NDEBUG
    size_t loadRequestSize = m_loadRequests.GetSize();
    for( size_t loadRequestIndex = 0; loadRequestIndex < loadRequestSize; ++loadRequestIndex )
    {
        if( !m_loadRequests.IsElementValid( loadRequestIndex ) )
        {
            continue;
        }

        LoadRequest* pRequest = m_loadRequests[ loadRequestIndex ];
        HELIUM_ASSERT( pRequest );
        HELIUM_ASSERT( pRequest->index != objectIndex );
        if( pRequest->index == objectIndex )
        {
            return Invalid< size_t >();
        }
    }
#endif

    LoadRequest* pRequest = m_loadRequestPool.Allocate();
    HELIUM_ASSERT( pRequest );
    HELIUM_ASSERT( !pRequest->spObject );
    pRequest->index = objectIndex;
    HELIUM_ASSERT( pRequest->linkTable.IsEmpty() );
    pRequest->spType = pType;
    HELIUM_ASSERT( !pRequest->spTemplate );
    HELIUM_ASSERT( !pRequest->spOwner );
    SetInvalid( pRequest->templateLoadId );
    SetInvalid( pRequest->ownerLoadId );
    SetInvalid( pRequest->persistentResourceDataLoadId );
    pRequest->pCachedObjectDataBuffer = NULL;
    pRequest->cachedObjectDataBufferSize = 0;

    pRequest->flags = 0;

    // If a fully-loaded object already exists with the same name, do not attempt to re-load the object (just mark
    // the request as complete).
    pRequest->spObject = GameObject::FindObject( path );

    GameObject* pObject = pRequest->spObject;
    if( pObject && pObject->IsFullyLoaded() )
    {
        pRequest->flags = LOAD_FLAG_PRELOADED;
    }
    else
    {
        HELIUM_ASSERT( !pObject || !pObject->GetAnyFlagSet( GameObject::FLAG_LOADED | GameObject::FLAG_LINKED ) );

        // Begin loading the template and owner objects.  Note that there isn't much reason to check for failure
        // until we tick this request, as we need to make sure any other load requests for the template/owner that
        // did succeed are properly synced anyway.
        GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
        HELIUM_ASSERT( pObjectLoader );

        if( rObjectData.templatePath.IsEmpty() )
        {
            // Make sure the template is fully loaded.
            GameObject* pTemplate = pType->GetTemplate();
            rObjectData.templatePath = pTemplate->GetPath();
            if( pTemplate->IsFullyLoaded() )
            {
                pRequest->spTemplate = pTemplate;
            }
            else
            {
                pRequest->templateLoadId = pObjectLoader->BeginLoadObject( rObjectData.templatePath );
            }
        }
        else
        {
            pRequest->templateLoadId = pObjectLoader->BeginLoadObject( rObjectData.templatePath );
        }

        GameObjectPath ownerPath = path.GetParent();
        if( ownerPath == m_packagePath )
        {
            // Easy check: if the owner is this package (which is likely), we don't need to load it.
            pRequest->spOwner = m_spPackage;
        }
        else if( !ownerPath.IsEmpty() )
        {
            pRequest->ownerLoadId = pObjectLoader->BeginLoadObject( ownerPath );
        }
    }

    size_t requestId = m_loadRequests.Add( pRequest );

    return requestId;
}

/// @copydoc PackageLoader::TryFinishLoadObject()
bool ArchivePackageLoader::TryFinishLoadObject(
    size_t requestId,
    GameObjectPtr& rspObject,
    DynArray< GameObjectLoader::LinkEntry >& rLinkTable )
{
    HELIUM_ASSERT( requestId < m_loadRequests.GetSize() );
    HELIUM_ASSERT( m_loadRequests.IsElementValid( requestId ) );

    LoadRequest* pRequest = m_loadRequests[ requestId ];
    HELIUM_ASSERT( pRequest );
    if( ( pRequest->flags & LOAD_FLAG_PRELOADED ) != LOAD_FLAG_PRELOADED )
    {
        return false;
    }

    // Sync on template and owner dependencies.
    GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
    HELIUM_ASSERT( pObjectLoader );

    if( IsValid( pRequest->templateLoadId ) )
    {
        if( !pObjectLoader->TryFinishLoad( pRequest->templateLoadId, pRequest->spTemplate ) )
        {
            return false;
        }

        SetInvalid( pRequest->templateLoadId );
    }

    if( IsValid( pRequest->ownerLoadId ) )
    {
        if( !pObjectLoader->TryFinishLoad( pRequest->ownerLoadId, pRequest->spOwner ) )
        {
            return false;
        }

        SetInvalid( pRequest->ownerLoadId );
    }

    // Sync on any in-flight async load requests for the cached object data.
    if( IsValid( pRequest->persistentResourceDataLoadId ) )
    {
        AsyncLoader& rAsyncLoader = AsyncLoader::GetStaticInstance();
        size_t bytesRead;
        if( !rAsyncLoader.TrySyncRequest( pRequest->persistentResourceDataLoadId, bytesRead ) )
        {
            return false;
        }

        SetInvalid( pRequest->persistentResourceDataLoadId );
    }

    DefaultAllocator().Free( pRequest->pCachedObjectDataBuffer );
    pRequest->pCachedObjectDataBuffer = NULL;
    pRequest->cachedObjectDataBufferSize = 0;

    rspObject = pRequest->spObject;
    GameObject* pObject = rspObject;
    if( pObject && ( pRequest->flags & LOAD_FLAG_ERROR ) )
    {
        pObject->SetFlags( GameObject::FLAG_BROKEN );
    }

    pRequest->spObject.Release();

    DynArray< LinkEntry >& rInternalLinkTable = pRequest->linkTable;
    size_t linkTableSize = rInternalLinkTable.GetSize();
    rLinkTable.Resize( 0 );
    rLinkTable.Reserve( linkTableSize );
    for( size_t linkIndex = 0; linkIndex < linkTableSize; ++linkIndex )
    {
        GameObjectLoader::LinkEntry* pEntry = rLinkTable.New();
        HELIUM_ASSERT( pEntry );
        pEntry->loadId = rInternalLinkTable[ linkIndex ].loadRequestId;
        pEntry->spObject.Release();
    }

    rInternalLinkTable.Resize( 0 );

    pRequest->spType.Release();
    pRequest->spTemplate.Release();
    pRequest->spOwner.Release();

    m_loadRequests.Remove( requestId );
    m_loadRequestPool.Release( pRequest );

    return true;
}

/// @copydoc PackageLoader::Tick()
void ArchivePackageLoader::Tick()
{
    MutexScopeLock scopeLock( m_accessLock );

    // Do nothing until pre-loading has been started.
    if( !m_startPreloadCounter )
    {
        return;
    }

    if( !m_preloadedCounter )
    {
        // Update package preloading.
        TickPreload();
    }
    else
    {
        // Process pending dependencies.
        TickLoadRequests();
    }
}

/// @copydoc PackageLoader::GetObjectCount()
size_t ArchivePackageLoader::GetObjectCount() const
{
    return m_objects.GetSize();
}

/// @copydoc PackageLoader::GetObjectPath()
GameObjectPath ArchivePackageLoader::GetObjectPath( size_t index ) const
{
    HELIUM_ASSERT( index < m_objects.GetSize() );

    return m_objects[ index ].objectPath;
}

/// Get the package managed by this loader.
///
/// @return  Associated package.
///
/// @see GetPackagePath()
Package* ArchivePackageLoader::GetPackage() const
{
    return m_spPackage;
}

/// Get the object path for the package managed by this loader.
///
/// @return  Path of the associated package.
///
/// @see GetPackage()
GameObjectPath ArchivePackageLoader::GetPackagePath() const
{
    return m_packagePath;
}

/// @copydoc PackageLoader::IsSourcePackageFile()
bool ArchivePackageLoader::IsSourcePackageFile() const
{
    return true;
}

/// @copydoc PackageLoader::GetFileTimestamp()
int64_t ArchivePackageLoader::GetFileTimestamp() const
{
    int64_t timestamp =  m_packageFilePath.ModifiedTime();

    return timestamp;
}

/// Update during the package preload process.
void ArchivePackageLoader::TickPreload()
{
    HELIUM_ASSERT( m_startPreloadCounter != 0 );
    HELIUM_ASSERT( m_preloadedCounter == 0 );

    // Test whether loading has completed.
    AsyncLoader& rAsyncLoader = AsyncLoader::GetStaticInstance();

    //PMDTODO: Handle the asynch load requests we gave for all the files
    // Walk through every load request
    for (size_t i = 0; i < m_fileReadRequests.GetSize();)
    {
        FileReadRequest &rRequest = m_fileReadRequests[i];
        HELIUM_ASSERT(rRequest.asyncLoadId);
        HELIUM_ASSERT(rRequest.pLoadBuffer);

        size_t bytes_read = 0;
        if (!rAsyncLoader.TrySyncRequest(rRequest.asyncLoadId, bytes_read))
        {
            // Havn't finished reading yet, move on to next entry
            ++i;
            continue;
        }

        HELIUM_ASSERT(bytes_read == rRequest.expectedSize);
        if( IsInvalid( bytes_read ) )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "ArchivePackageLoader: Failed to read the contents of async load request \"%d\".\n" ),
                rRequest.asyncLoadId );
        }
        else if( bytes_read != rRequest.expectedSize)
        {
            HELIUM_TRACE(
                TRACE_WARNING,
                ( TXT( "ArchivePackageLoader: Attempted to read %" ) TPRIuSZ TXT( " bytes from package file \"%s\", " )
                TXT( "but only %" ) TPRIuSZ TXT( " bytes were read.\n" ) ),
                rRequest.expectedSize,
                bytes_read );
        }
        else
        {
            // TODO: Read it in
            //tstringstream ss;
            //ss << (tchar_t *)rRequest.pLoadBuffer;
            tstring str;
            str.assign((tchar_t *)rRequest.pLoadBuffer, rRequest.expectedSize / sizeof(tchar_t));
            Helium::Reflect::ObjectPtr object = Reflect::ArchiveXML::FromString(str);
        }

        // We're finished with this load, so deallocate memory and get rid of the request
        DefaultAllocator().Free( rRequest.pLoadBuffer );
        rRequest.pLoadBuffer = NULL;
        SetInvalid(rRequest.asyncLoadId);
        m_fileReadRequests.RemoveSwap(i);

    }

    // Wait for the parent package to finish loading.
    GameObjectPtr spParentPackage;
    if( IsValid( m_parentPackageLoadId ) )
    {
        GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
        HELIUM_ASSERT( pObjectLoader );
        if( !pObjectLoader->TryFinishLoad( m_parentPackageLoadId, spParentPackage ) )
        {
            return;
        }

        SetInvalid( m_parentPackageLoadId );

        // Package loading should not fail.  If it does, this is a sign of a potentially serious issue.
        HELIUM_ASSERT( spParentPackage );
    }

    // Create the package object if it does not yet exist.
    Package* pPackage = m_spPackage;
    if( !pPackage )
    {
        HELIUM_ASSERT( spParentPackage ? !m_packagePath.GetParent().IsEmpty() : m_packagePath.GetParent().IsEmpty() );
        HELIUM_VERIFY( GameObject::Create< Package >( m_spPackage, m_packagePath.GetName(), spParentPackage ) );
        pPackage = m_spPackage;
        HELIUM_ASSERT( pPackage );
        pPackage->SetLoader( this );
    }

    HELIUM_ASSERT( pPackage->GetLoader() == this );
/*
#if HELIUM_EDITOR
    // Add all resource objects that exist in the package directory.
    DynArray< ResourceHandler* > resourceHandlers;
    ResourceHandler::GetAllResourceHandlers( resourceHandlers );
    size_t resourceHandlerCount = resourceHandlers.GetSize();

    Path packageDirectoryPath;

    if ( !File::GetDataDirectory( packageDirectoryPath ) )
    {
        HELIUM_TRACE( TRACE_ERROR, TXT( "ArchivePackageLoader::TickPreload(): Could not get data directory.\n" ) );
        return;
    }

    packageDirectoryPath += m_packagePath.ToFilePathString().GetData();
    packageDirectoryPath += TXT("/");

    Directory packageDirectory( packageDirectoryPath );

    for( ; !packageDirectory.IsDone(); packageDirectory.Next() )
    {
        const DirectoryItem& item = packageDirectory.GetItem();

        if ( !item.m_Path.IsFile() )
        {
            continue;
        }

        // Make sure an object entry doesn't already exist for the file.
        String objectNameString( *item.m_Path );

        size_t pathSeparatorLocation = objectNameString.FindReverse( TXT( '/' ) );
        if( IsValid( pathSeparatorLocation ) )
        {
            objectNameString.Substring( objectNameString, pathSeparatorLocation + 1 );
        }

        Name objectName( objectNameString );
        size_t objectCount = m_objects.GetSize();
        size_t objectIndex;
        for( objectIndex = 0; objectIndex < objectCount; ++objectIndex )
        {
            SerializedObjectData& rObjectData = m_objects[ objectIndex ];
            if( rObjectData.objectPath.GetName() == objectName &&
                rObjectData.objectPath.GetParent() == m_packagePath )
            {
                break;
            }
        }

        if( objectIndex < objectCount )
        {
            continue;
        }

        // Check the extension to see if the file is supported by one of the resource handlers.
        ResourceHandler* pBestHandler = NULL;
        size_t bestHandlerExtensionLength = 0;

        for( size_t handlerIndex = 0; handlerIndex < resourceHandlerCount; ++handlerIndex )
        {
            ResourceHandler* pHandler = resourceHandlers[ handlerIndex ];
            HELIUM_ASSERT( pHandler );

            const tchar_t* const* ppExtensions;
            size_t extensionCount;
            pHandler->GetSourceExtensions( ppExtensions, extensionCount );
            HELIUM_ASSERT( ppExtensions || extensionCount == 0 );

            for( size_t extensionIndex = 0; extensionIndex < extensionCount; ++extensionIndex )
            {
                const tchar_t* pExtension = ppExtensions[ extensionIndex ];
                HELIUM_ASSERT( pExtension );

                size_t extensionLength = StringLength( pExtension );
                if( extensionLength > bestHandlerExtensionLength && objectNameString.EndsWith( pExtension ) )
                {
                    pBestHandler = pHandler;
                    bestHandlerExtensionLength = extensionLength;

                    break;
                }
            }
        }

        if( pBestHandler )
        {
            // File extension matches a supported source asset type, so add it to the object list.
            const GameObjectType* pResourceType = pBestHandler->GetResourceType();
            HELIUM_ASSERT( pResourceType );

            HELIUM_TRACE(
                TRACE_DEBUG,
                ( TXT( "ArchivePackageLoader: Registered source asset file \"%s\" as as instance of resource " )
                TXT( "type \"%s\" in package \"%s\".\n" ) ),
                *objectNameString,
                *pResourceType->GetName(),
                *m_packagePath.ToString() );

            SerializedObjectData* pObjectData = m_objects.New();
            HELIUM_ASSERT( pObjectData );
            HELIUM_VERIFY( pObjectData->objectPath.Set( objectName, false, m_packagePath ) );
            pObjectData->typeName = pResourceType->GetName();
            pObjectData->templatePath.Clear();
        }
    }

#endif  // HELIUM_EDITOR
    */

    // Package preloading is now complete.
    pPackage->SetFlags( GameObject::FLAG_PRELOADED | GameObject::FLAG_LINKED );
    pPackage->ConditionalFinalizeLoad();

    AtomicExchangeRelease( m_preloadedCounter, 1 );
}

/// Update load processing of object load requests.
void ArchivePackageLoader::TickLoadRequests()
{
    size_t loadRequestCount = m_loadRequests.GetSize();
    for( size_t loadRequestIndex = 0; loadRequestIndex < loadRequestCount; ++loadRequestIndex )
    {
        if( !m_loadRequests.IsElementValid( loadRequestIndex ) )
        {
            continue;
        }

        LoadRequest* pRequest = m_loadRequests[ loadRequestIndex ];
        HELIUM_ASSERT( pRequest );

        if( !( pRequest->flags & LOAD_FLAG_PROPERTY_PRELOADED ) )
        {
            if( !TickDeserialize( pRequest ) )
            {
                continue;
            }
        }

        if( !( pRequest->flags & LOAD_FLAG_PERSISTENT_RESOURCE_PRELOADED ) )
        {
            if( !TickPersistentResourcePreload( pRequest ) )
            {
                continue;
            }
        }
    }
}

/// Update processing of object property preloading for a given load request.
///
/// @param[in] pRequest  Load request to process.
///
/// @return  True if object property preloading for the given load request has completed, false if not.
bool ArchivePackageLoader::TickDeserialize( LoadRequest* pRequest )
{
    HELIUM_ASSERT( pRequest );
    HELIUM_ASSERT( !( pRequest->flags & LOAD_FLAG_PROPERTY_PRELOADED ) );

    GameObject* pObject = pRequest->spObject;

    HELIUM_ASSERT( pRequest->index < m_objects.GetSize() );
    SerializedObjectData& rObjectData = m_objects[ pRequest->index ];

    // Wait for the template and owner objects to load.
    GameObjectLoader* pObjectLoader = GameObjectLoader::GetStaticInstance();
    HELIUM_ASSERT( pObjectLoader );

    if( !rObjectData.templatePath.IsEmpty() )
    {
        if( IsValid( pRequest->templateLoadId ) )
        {
            if( !pObjectLoader->TryFinishLoad( pRequest->templateLoadId, pRequest->spTemplate ) )
            {
                return false;
            }

            SetInvalid( pRequest->templateLoadId );
        }

        if( !pRequest->spTemplate )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "ArchivePackageLoader: Failed to load template object for \"%s\".\n" ),
                *rObjectData.objectPath.ToString() );

            if( pObject )
            {
                pObject->SetFlags( GameObject::FLAG_PRELOADED | GameObject::FLAG_LINKED );
                pObject->ConditionalFinalizeLoad();
            }

            pRequest->flags |= LOAD_FLAG_PRELOADED | LOAD_FLAG_ERROR;

            return true;
        }
    }

    HELIUM_ASSERT( IsInvalid( pRequest->templateLoadId ) );
    GameObject* pTemplate = pRequest->spTemplate;

    GameObjectPath ownerPath = rObjectData.objectPath.GetParent();
    if( !ownerPath.IsEmpty() )
    {
        if( IsValid( pRequest->ownerLoadId ) )
        {
            if( !pObjectLoader->TryFinishLoad( pRequest->ownerLoadId, pRequest->spOwner ) )
            {
                return false;
            }

            SetInvalid( pRequest->ownerLoadId );
        }

        if( !pRequest->spOwner )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "ArchivePackageLoader: Failed to load owner object for \"%s\".\n" ),
                *rObjectData.objectPath.ToString() );

            if( pObject )
            {
                pObject->SetFlags( GameObject::FLAG_PRELOADED | GameObject::FLAG_LINKED );
                pObject->ConditionalFinalizeLoad();
            }

            pRequest->flags |= LOAD_FLAG_PRELOADED | LOAD_FLAG_ERROR;

            return true;
        }
    }

    HELIUM_ASSERT( IsInvalid( pRequest->ownerLoadId ) );
    GameObject* pOwner = pRequest->spOwner;

    GameObjectType* pType = pRequest->spType;
    HELIUM_ASSERT( pType );

    HELIUM_ASSERT( !pOwner || pOwner->IsFullyLoaded() );
    HELIUM_ASSERT( !pTemplate || pTemplate->IsFullyLoaded() );

    // If we already had an existing object, make sure the type and template match.
    if( pObject )
    {
        const GameObjectType* pExistingType = pObject->GetGameObjectType();
        HELIUM_ASSERT( pExistingType );
        if( pExistingType != pType )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                ( TXT( "ArchivePackageLoader: Cannot load \"%s\" using the existing object as the types do not match " )
                TXT( "(existing type: \"%s\"; serialized type: \"%s\".\n" ) ),
                *rObjectData.objectPath.ToString(),
                *pExistingType->GetName(),
                *pType->GetName() );

            pObject->SetFlags( GameObject::FLAG_PRELOADED | GameObject::FLAG_LINKED );
            pObject->ConditionalFinalizeLoad();

            pRequest->flags |= LOAD_FLAG_PRELOADED | LOAD_FLAG_ERROR;

            return true;
        }
    }
    else
    {
        // Create the object.
        bool bCreateResult = GameObject::CreateObject(
            pRequest->spObject,
            pType,
            rObjectData.objectPath.GetName(),
            pOwner,
            pTemplate );
        if( !bCreateResult )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                TXT( "ArchivePackageLoader: Failed to create \"%s\" during loading.\n" ),
                *rObjectData.objectPath.ToString() );

            pRequest->flags |= LOAD_FLAG_PRELOADED | LOAD_FLAG_ERROR;

            return true;
        }

        pObject = pRequest->spObject;
        HELIUM_ASSERT( pObject );
    }

    // Load the object properties.
    // PMDTODO: Fix this
    //Deserializer deserializer;
    //deserializer.Prepare( &rObjectData, &pRequest->linkTable );
    //bool bResult = deserializer.Serialize( pObject );
    bool bResult = false;

    pRequest->flags |= LOAD_FLAG_PROPERTY_PRELOADED;

    if( !bResult )
    {
        HELIUM_TRACE(
            TRACE_ERROR,
            TXT( "ArchivePackageLoader: Deserialization of object \"%s\" failed.\n" ),
            *rObjectData.objectPath.ToString() );

        // Clear out object references (object can now be considered fully loaded as well).
        NullLinker().Serialize( pObject );
        pObject->SetFlags( GameObject::FLAG_PRELOADED | GameObject::FLAG_LINKED );
        pObject->ConditionalFinalizeLoad();

        pRequest->flags |= LOAD_FLAG_ERROR;

        return true;
    }

    // If the object is a resource (not including the default template object for resource types), attempt to begin
    // loading any existing persistent resource data stored in the object cache.
    if( !pObject->IsDefaultTemplate() )
    {
        Resource* pResource = Reflect::SafeCast< Resource >( pObject );
        if( pResource )
        {
            Name objectCacheName = pObjectLoader->GetCacheName();
            CacheManager& rCacheManager = CacheManager::GetStaticInstance();

            Cache* pCache = rCacheManager.GetCache( objectCacheName );
            HELIUM_ASSERT( pCache );
            pCache->EnforceTocLoad();

            const Cache::Entry* pEntry = pCache->FindEntry( rObjectData.objectPath, 0 );
            if( pEntry && pEntry->size != 0 )
            {
                HELIUM_ASSERT( IsInvalid( pRequest->persistentResourceDataLoadId ) );
                HELIUM_ASSERT( !pRequest->pCachedObjectDataBuffer );

                pRequest->pCachedObjectDataBuffer =
                    static_cast< uint8_t* >( DefaultAllocator().Allocate( pEntry->size ) );
                HELIUM_ASSERT( pRequest->pCachedObjectDataBuffer );
                pRequest->cachedObjectDataBufferSize = pEntry->size;

                AsyncLoader& rAsyncLoader = AsyncLoader::GetStaticInstance();
                pRequest->persistentResourceDataLoadId = rAsyncLoader.QueueRequest(
                    pRequest->pCachedObjectDataBuffer,
                    pCache->GetCacheFileName(),
                    pEntry->offset,
                    pEntry->size );
                HELIUM_ASSERT( IsValid( pRequest->persistentResourceDataLoadId ) );
            }
        }
    }

    if( IsInvalid( pRequest->persistentResourceDataLoadId ) )
    {
        // No persistent resource data needs to be loaded.
        pObject->SetFlags( GameObject::FLAG_PRELOADED );
        pRequest->flags |= LOAD_FLAG_PERSISTENT_RESOURCE_PRELOADED;
    }

    // GameObject is now preloaded.
    return true;
}

/// Update processing of persistent resource data loading for a given load request.
///
/// @param[in] pRequest  Load request to process.
///
/// @return  True if persistent resource data loading for the given load request has completed, false if not.
bool ArchivePackageLoader::TickPersistentResourcePreload( LoadRequest* pRequest )
{
    HELIUM_ASSERT( pRequest );
    HELIUM_ASSERT( !( pRequest->flags & LOAD_FLAG_PERSISTENT_RESOURCE_PRELOADED ) );

    Resource* pResource = Reflect::AssertCast< Resource >( pRequest->spObject.Get() );
    HELIUM_ASSERT( pResource );

    // Wait for the cached data load to complete.
    AsyncLoader& rAsyncLoader = AsyncLoader::GetStaticInstance();

    size_t bytesRead = 0;
    HELIUM_ASSERT( IsValid( pRequest->persistentResourceDataLoadId ) );
    if( !rAsyncLoader.TrySyncRequest( pRequest->persistentResourceDataLoadId, bytesRead ) )
    {
        return false;
    }

    SetInvalid( pRequest->persistentResourceDataLoadId );

    if( bytesRead != pRequest->cachedObjectDataBufferSize )
    {
        HELIUM_TRACE(
            TRACE_WARNING,
            ( TXT( "ArchivePackageLoader: Requested load of %" ) TPRIu32 TXT( " bytes from cached object data for " )
            TXT( "\"%s\", but only %" ) TPRIuSZ TXT( " bytes were read.\n" ) ),
            pRequest->cachedObjectDataBufferSize,
            *pResource->GetPath().ToString(),
            bytesRead );
    }

    // Make sure we read enough bytes to cover the object property data size.
    if( bytesRead < sizeof( uint32_t ) )
    {
        HELIUM_TRACE(
            TRACE_ERROR,
            ( TXT( "ArchivePackageLoader: Not enough bytes read of cached object data \"%s\" from which to parse the " )
            TXT( "property stream size.\n" ) ),
            *pResource->GetPath().ToString() );

        pRequest->flags |= LOAD_FLAG_ERROR;
    }
    else
    {
        // Skip over the object property data.
        uint8_t* pCachedObjectData = pRequest->pCachedObjectDataBuffer;
        HELIUM_ASSERT( pCachedObjectData );

        uint32_t propertyDataSize = *reinterpret_cast< uint32_t* >( pCachedObjectData );

        size_t byteSkipCount = sizeof( propertyDataSize ) + propertyDataSize;
        if( byteSkipCount > bytesRead )
        {
            HELIUM_TRACE(
                TRACE_ERROR,
                ( TXT( "ArchivePackageLoader: Cached persistent resource data for \"%s\" extends past the end of the " )
                TXT( "cached data stream.\n" ) ),
                *pResource->GetPath().ToString() );

            pRequest->flags |= LOAD_FLAG_ERROR;
        }
        else
        {
            pCachedObjectData += byteSkipCount;
            size_t bytesRemaining = bytesRead - byteSkipCount;

            // Make sure we have enough bytes at the end for the resource sub-data count.
            if( bytesRemaining < sizeof( uint32_t ) )
            {
                HELIUM_TRACE(
                    TRACE_ERROR,
                    ( TXT( "ArchivePackageLoader: Not enough space is reserved in the cached persistent resource " )
                    TXT( "data stream for \"%s\" for the resource sub-data count.\n" ) ),
                    *pResource->GetPath().ToString() );

                pRequest->flags |= LOAD_FLAG_ERROR;
            }
            else
            {
                bytesRemaining -= sizeof( uint32_t );

                // Deserialize the persistent resource data.
                // PMDTODO: Fix this
                //BinaryDeserializer deserializer;
                //deserializer.Prepare( pCachedObjectData, bytesRemaining );

                //deserializer.BeginSerialize();
                //pResource->SerializePersistentResourceData( deserializer );
                //if( !deserializer.EndSerialize() )
                //{
                //    HELIUM_TRACE(
                //        TRACE_ERROR,
                //        ( TXT( "ArchivePackageLoader: Attempted to read past the end of the cached data stream when " )
                //        TXT( "deserializing persistent resource data for \"%s\".\n" ) ),
                //        *pResource->GetPath().ToString() );

                //    pRequest->flags |= LOAD_FLAG_ERROR;
                //}
            }
        }
    }

    DefaultAllocator().Free( pRequest->pCachedObjectDataBuffer );
    pRequest->pCachedObjectDataBuffer = NULL;
    pRequest->cachedObjectDataBufferSize = 0;

    pResource->SetFlags( GameObject::FLAG_PRELOADED );

    pRequest->flags |= LOAD_FLAG_PERSISTENT_RESOURCE_PRELOADED;

    return true;
}