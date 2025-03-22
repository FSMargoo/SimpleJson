# SimpeJSON（sJSON）

## 简介
SimpleJSon（简称 sJSON）是一个只有一个 hpp 文件的轻量级 C++ JSON 库，使用方法优雅，支持 C11 及以上的 C++ 标准。

## 实例
下面是一个使用 sJSON 解析 JSON 的实例：
```cpp
#include "sJson.hpp"

int main()
{
	sJSONParser Parser((LR"({
    "sites": [
        { "name":"codeforce" , "url":"www.codeforces.com", "rank":3 }, 
        { "name":"google" , "url":"www.google.com", "rank":2 }, 
        { "name":"github" , "url":"www.github.com", "rank":1 }
    ]
})"));
	// ���� JSON
	auto Root = Parser.Parse();

	// ���л� JSON ����
	printf("JSON in format : %ws\n", sJSONWriter::WriteJSON(Root).c_str());
	printf("JSON in not format : %ws\n\n", sJSONWriter::WriteJSON(Root, false).c_str());

	// ��ȡ JSON �ļ�
	for (auto Object = Root[L"sites"].ArrayBegin(); Object != Root[L"sites"].ArrayEnd(); ++Object)
	{
		auto ObjectExpand = sJSONElementFinder((*Object)->To<sJSONObject *>());
		for (auto Instance : ObjectExpand)
		{
			printf("<%ws, ", Instance.first.c_str());
			if (sJSONwstring::Equal(**Instance.second))
			{
				printf("%ws> ", (**Instance.second->To<sJSONwstring*>()).c_str());
			}
			if (sJSONInt::Equal(**Instance.second))
			{
				printf("%d> ", **Instance.second->To<sJSONInt *>());
			}
		}
		printf("\n");
	}

	return 0;
}
```
运行上面的代码，最终效果如下：
```
JSON in format : {
        "sites":[
                {
                        "name":"codeforce",
                        "rank":3,
                        "url":"www.codeforces.com"
                },
                {
                        "name":"google",
                        "rank":2,
                        "url":"www.google.com"
                },
                {
                        "name":"github",
                        "rank":1,
                        "url":"www.github.com"
                }
        ]
}
JSON in not format : {"sites":[{"name":"codeforce","rank":3,"url":"www.codeforces.com"},{"name":"google","rank":2,"url":"www.google.com"},{"name":"github","rank":1,"url":"www.github.com"}]}

<name, codeforce> <rank, 3> <url, www.codeforces.com>
<name, google> <rank, 2> <url, www.google.com>
<name, github> <rank, 1> <url, www.github.com>
```