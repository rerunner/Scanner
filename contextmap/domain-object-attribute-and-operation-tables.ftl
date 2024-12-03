<#--
 Makro that takes a SimpleDomainObject and lists all operations and attributes in two tables.
 -->
<#macro renderDomainObjectOperationsAndAttributes simpleDomainObject>
    <#if simpleDomainObject.operations?has_content>
        <#list simpleDomainObject.operations as op>
        <#if op.returnType?has_content><#if getType(op.returnType)?has_content><#if op.returnType.type?has_content><#if op.returnType.collectionType?has_content && op.returnType.collectionType.name() == "LIST">std::list<${op.returnType.type}><#else>${getType(op.returnType)}</#if><#else>std::shared_ptr<${getType(op.returnType)}></#if></#if><#else><#if op.name == simpleDomainObject.name>/*constructor*/<#else>void</#if></#if> ${op.name}(<#if op.parameters?has_content><#list op.parameters as p><#if getType(p.parameterType)?has_content> ${getType(p.parameterType)} ${p.name}<#if p?has_next>,<#else></#if></#if></#list><#else>void</#if>);
        <#--${op.name} ${op.parameters?join(", ")}-->
        </#list>
    </#if>
</#macro>
