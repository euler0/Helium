#pragma once

#include "Pipeline/API.h"
#include "Foundation/Automation/Attribute.h"
#include "Foundation/Document/Document.h"
#include "Foundation/Reflect/DOM.h"

namespace Helium
{
    class PIPELINE_API Project : public Reflect::Element
    {
    public:
        REFLECT_DECLARE_CLASS( Project, Reflect::Element );

        Project( const Path& path = TXT( "" ) );
        virtual ~Project();

        void ConnectDocument( Document* document );
        void DisconnectDocument( const Document* document );

        // Document and DocumentManager Events
        void OnDocumentOpened( const DocumentEventArgs& args );
        void OnDocumenClosed( const DocumentEventArgs& args );

        bool Serialize() const;

        const std::set< Path >& Paths()
        {
            return m_Paths;
        }

        void AddPath( const Path& path )
        {
            std::pair< std::set< Path >::iterator, bool > result = m_Paths.insert( path );
            if ( result.second )
            {
                e_PathAdded.Raise( path );
            }
        }

        void RemovePath( const Path& path )
        {
            std::set< Path >::iterator itr = m_Paths.find( path );
            if ( itr != m_Paths.end() )
            {
                m_Paths.erase( itr );
                e_PathRemoved.Raise( path );
            }
        }

    public:
        Helium::Attribute< Path >    a_Path;
        Helium::Event< const Path& > e_PathAdded;
        Helium::Event< const Path& > e_PathRemoved;

        mutable DocumentObjectChangedSignature::Event e_HasChanged;

    protected:
        std::set< Path > m_Paths;

        void OnDocumentSave( const DocumentEventArgs& args );
        void OnDocumentPathChanged( const DocumentPathChangedArgs& args );
        void OnChildDocumentPathChanged( const DocumentPathChangedArgs& args );

    public:
        static void EnumerateClass( Reflect::Compositor< This >& comp )
        {
            comp.AddField( &This::a_Path, "Path", Reflect::FieldFlags::Discard );
            comp.AddField( &This::m_Paths, "m_Paths" );
        }
    };

    typedef Helium::StrongPtr<Project> ProjectPtr;
}