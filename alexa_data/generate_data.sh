#!/bin/bash
for line in `cat Top1000.csv`;
do
	java UrlInfo AKIAJYAW7BD3ZGBD2YMQ lWEOHjZNtlHOUbv9BVATmhlNrkBZGbr7kaXx5Dvm $line >> SiteInfo.xml
done
