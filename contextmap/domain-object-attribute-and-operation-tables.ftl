<#--
 Makro that takes a SimpleDomainObject and lists all operations and attributes in two tables.
 -->
<#macro renderDomainObjectOperationsAndAttributes simpleDomainObject>
    <#if simpleDomainObject.operations?has_content>
        <#list simpleDomainObject.operations as op>
        <#if op.returnType?has_content><#if getType(op.returnType)?has_content>${getType(op.returnType)}</#if><#else>void</#if> ${op.name}(<#if op.parameters?has_content><#list op.parameters as p><#if getType(p.parameterType)?has_content> ${getType(p.parameterType)} ${p.name}</#if></#list><#else>void</#if>);
        <#--${op.name} ${op.parameters?join(", ")}-->
        </#list>
    </#if>
</#macro>
